/*
 * Copyright (C) by Oleksandr Zolotov <alex@nextcloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "thumbnailprovider.h"
#include "common/cfapishellextensionsipcconstants.h"
#include <shlwapi.h>
#include <QObject>
#include <QPixmap>
#include <QJsonDocument>
#include <ntstatus.h>
#include <atlimage.h>

namespace {
// we don't want to block the Explorer for too long (default is 30K, so we'd keep it at 10K, except QLocalSocket::waitForDisconnected())
constexpr auto socketTimeoutMs = 10000;
}

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT HBITMAP qt_imageToWinHBITMAP(const QImage &imageIn, int hbitmapFormat = 0);
QT_END_NAMESPACE

ThumbnailProvider::ThumbnailProvider()
    : _referenceCount(1)
{
}

IFACEMETHODIMP ThumbnailProvider::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(ThumbnailProvider, IInitializeWithItem),
        QITABENT(ThumbnailProvider, IThumbnailProvider),
        {0},
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) ThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP_(ULONG) ThumbnailProvider::Release()
{
    const auto refCount = InterlockedDecrement(&_referenceCount);
    if (refCount == 0) {
        delete this;
    }
    return refCount;
}

IFACEMETHODIMP ThumbnailProvider::Initialize(_In_ IShellItem *item, _In_ DWORD mode)
{
    HRESULT hresult = item->QueryInterface(__uuidof(_shellItem), reinterpret_cast<void **>(&_shellItem));
    if (FAILED(hresult)) {
        return hresult;
    }

    LPWSTR pszName = NULL;
    hresult = _shellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszName);
    if (FAILED(hresult)) {
        return hresult;
    }

    _shellItemPath = QString::fromWCharArray(pszName);

    return S_OK;
}

HBITMAP hBitmapFromBuffer(const std::vector<unsigned char> const &data)
{
    if (data.empty()) {
        _com_issue_error(E_INVALIDARG);
    }

    auto const stream{::SHCreateMemStream(&data[0], static_cast<UINT>(data.size()))};
    if (!stream) {
        _com_issue_error(E_OUTOFMEMORY);
    }
    _COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));
    IStreamPtr streamPtr{stream, false};

    CImage img{};
    _com_util::CheckError(img.Load(streamPtr));
    return img.Detach();
}

IFACEMETHODIMP ThumbnailProvider::GetThumbnail(_In_ UINT cx, _Out_ HBITMAP *bitmap, _Out_ WTS_ALPHATYPE *alphaType)
{
    *bitmap = nullptr;
    *alphaType = WTSAT_UNKNOWN;

    const auto disconnectSocketFromServer = [this]() {
        const auto isConnectedOrConnecting = _localSocket.state() == QLocalSocket::ConnectedState || _localSocket.state() == QLocalSocket::ConnectingState;
        if (isConnectedOrConnecting) {
            _localSocket.disconnectFromServer();
            const auto isNotConnected = _localSocket.state() == QLocalSocket::UnconnectedState || _localSocket.state() == QLocalSocket::ClosingState;
            return isNotConnected || _localSocket.waitForDisconnected();
        }
        return true;
    };

    const auto connectSocketToServer = [this, &disconnectSocketFromServer](const QString &serverName) {
        if (!disconnectSocketFromServer()) {
            return false;
        }
        _localSocket.setServerName(serverName);
        _localSocket.connectToServer();
        return _localSocket.state() == QLocalSocket::ConnectedState || _localSocket.waitForConnected(socketTimeoutMs);
    };

    const auto sendMessageAndReadyRead = [this](const QByteArray &message) {
        _localSocket.write(message);
        return _localSocket.waitForBytesWritten(socketTimeoutMs) && _localSocket.waitForReadyRead(socketTimeoutMs);
    };

    // #1 Connect to main server and get the name of a server for the current syncroot
    if (!connectSocketToServer(CfApiShellExtensions::IpcMainServerName)) {
        return E_FAIL;
    }

    // send the file path so the main server will decide which sync root we are working with
    const auto messageRequestThumbnailForFile = QJsonDocument::fromVariant(
        QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestKey,
            QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestFilePathKey, _shellItemPath},
                {CfApiShellExtensions::Protocol::ThumbnailProviderRequestFileSizeKey,
                    QVariantMap{{"x", cx}, {"y", cx}}}}}}).toJson(QJsonDocument::Compact);

    if (!sendMessageAndReadyRead(messageRequestThumbnailForFile)) {
        return E_FAIL;
    }

    // the main server will start the new server for a specific syncroot and reply with its name
    const auto receivedSyncrootServerNameMessage = QJsonDocument::fromJson(_localSocket.readAll()).toVariant().toMap();
    const auto serverNameReceived =
        receivedSyncrootServerNameMessage.value(CfApiShellExtensions::Protocol::ServerNameKey).toString();

    if (serverNameReceived.isEmpty()) {
        disconnectSocketFromServer();
        return E_FAIL;
    }

    // #2 Connect to the current syncroot folder's server
    if (!connectSocketToServer(serverNameReceived)) {
        return E_FAIL;
    }

    // #3 Get a thumbnail format from the current syncroot folder's server and request a thumbnail of size (x, y) for a file _shellItemPath
    if (!sendMessageAndReadyRead(messageRequestThumbnailForFile)) {
        return E_FAIL;
    }

    const auto receivedThumbnailFormatMessage = QJsonDocument::fromJson(_localSocket.readAll()).toVariant().toMap();
    auto thumbnailFormatReceived = receivedThumbnailFormatMessage.value(CfApiShellExtensions::Protocol::ThumbnailFormatKey).toString();
    // the format (JPG, PNG, GIF) will get detected based on what the file server will return to a local server of the current syncroot
    if (thumbnailFormatReceived.isEmpty()
        || thumbnailFormatReceived == CfApiShellExtensions::Protocol::ThumbnailFormatTagEmptyValue) {
        disconnectSocketFromServer();
        return E_FAIL;
    }
    const auto hasAlphaChannel = receivedThumbnailFormatMessage.value(CfApiShellExtensions::Protocol::ThumbnailAlphaKey).toBool();

    // #4 Notify the current syncroot folder's server that we are ready to receive a thumbnail data (QByteArray)
    const auto readyToAceptThumbnailMessage = QJsonDocument::fromVariant(
        QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestKey,
            QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestAcceptReadyKey,
                true}}}}).toJson(QJsonDocument::Compact);

    if (!sendMessageAndReadyRead(readyToAceptThumbnailMessage)) {
        return E_FAIL;
    }

    // #5 Read the thumbnail data from the current syncroot folder's server (read all as the thumbnail size is usually less than 1MB)
    const auto bitmapReceived = _localSocket.readAll();
    disconnectSocketFromServer();

    if (bitmapReceived.isEmpty()) {
        disconnectSocketFromServer();
        return E_FAIL;
    }

    std::vector<unsigned char> bufferToCompress(bitmapReceived.begin(), bitmapReceived.end());

    try {
        *bitmap = hBitmapFromBuffer(bufferToCompress);
        *alphaType = hasAlphaChannel ? WTSAT_ARGB : WTSAT_RGB;
        if (!bitmap) {
            return E_FAIL;
        }
    } catch (_com_error exc) {
        return E_FAIL;
    }
    
    return S_OK;
}