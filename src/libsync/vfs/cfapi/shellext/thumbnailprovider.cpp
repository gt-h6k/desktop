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
#include "logger.h"

#include <Shlguid.h>
#include <locale>
#include <codecvt>

#include "config.h"
#include "common/cfapishellextensionsipcconstants.h"

#include <QObject>
#include <QPixmap>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT HBITMAP qt_imageToWinHBITMAP(const QImage &imageIn, int hbitmapFormat = 0);
QT_END_NAMESPACE

using convert_type = std::codecvt_utf8<wchar_t>;

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

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
    ULONG cRef = InterlockedDecrement(&_referenceCount);
    if (!cRef) {
        delete this;
    }
    return cRef;
}

IFACEMETHODIMP ThumbnailProvider::Initialize(_In_ IShellItem *item, _In_ DWORD mode)
{
    // MessageBox(NULL, L"Attach to DLL", L"Attach Now", MB_OK);

    try {
        throw_if_fail((item->QueryInterface(__uuidof(_itemDest), reinterpret_cast<void **>(&_itemDest))));

        LPWSTR pszName = NULL;
        throw_if_fail(_itemDest->GetDisplayName(SIGDN_FILESYSPATH, &pszName));

        _itemPath = pszName; 
/*
        std::wstring sourceItem = L"D:\\work\\cloud-sample\\server\\bird-g272205618_640.jpg";


        IShellItem *pShellItem = 0;

        throw_if_fail((SHCreateItemFromParsingName(
            sourceItem.data(), NULL, __uuidof(_itemSrc), reinterpret_cast<void **>(&_itemSrc))));
            */
        std::wstring_convert<convert_type, wchar_t> converter;
        std::string converted_str = converter.to_bytes(std::wstring(pszName));

        writeLog(std::string("ThumbnailProvider::Initialize: pszName") + converted_str);
    } catch (_com_error exc) {
        std::wstring_convert<convert_type, wchar_t> converter;
        std::string converted_str = converter.to_bytes(std::wstring(exc.ErrorMessage()));
        writeLog(std::string("Error: ") + std::to_string(exc.Error()) + std::string(" ") + converted_str);
        return exc.Error();
    }

    return S_OK;
}

// IThumbnailProvider
IFACEMETHODIMP ThumbnailProvider::GetThumbnail(_In_ UINT cx, _Out_ HBITMAP *bitmap, _Out_ WTS_ALPHATYPE *alphaType)
{
   // MessageBox(NULL, L"Attach to DLL", L"Attach Now", MB_OK);
    // Open a handle to the file
    writeLog("ThumbnailProvider::GetThumbnail...");
    // Retrieve thumbnails of the placeholders on demand by delegating to the thumbnail of the source items.
    *bitmap = nullptr;
    *alphaType = WTSAT_UNKNOWN;

    const auto disconnectSocketFromServer = [this]() {
        if (_localSocket.state() == QLocalSocket::ConnectedState
            || _localSocket.state() == QLocalSocket::ConnectingState) {
            _localSocket.disconnectFromServer();
            if (_localSocket.state() != QLocalSocket::UnconnectedState) {
                if (!_localSocket.waitForDisconnected()) {
                    return false;
                }
            }
        }
        return true;
    };

    const auto connectSocketToServer = [this, &disconnectSocketFromServer](const QString &serverName) {
        if (!disconnectSocketFromServer()) {
            return false;
        }

        _localSocket.setServerName(serverName);
        _localSocket.connectToServer();
        if (!_localSocket.waitForConnected()) {
            return false;
        }
        return true;
    };

    const auto sendMessage = [this](const QByteArray &message) {
        _localSocket.write(message);
        return _localSocket.waitForBytesWritten() && _localSocket.waitForReadyRead();
    };

    // #1 Connect to main server and get the name of a server for the current syncroot
    if (!connectSocketToServer(CfApiShellExtensions::IpcMainServerName)) {
        return E_FAIL;
    }
    
    const auto messageRequestThumbnailForFile = QJsonDocument::fromVariant(
        QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestKey, QVariantMap{
                {CfApiShellExtensions::Protocol::ThumbnailProviderRequestFilePathKey, QString::fromStdWString(_itemPath)},
                {CfApiShellExtensions::Protocol::ThumbnailProviderRequestFileSizeKey, QVariantMap { {"x", cx}, {"y", cx} }}
            }
        }}).toJson(QJsonDocument::Compact);

    if (!sendMessage(messageRequestThumbnailForFile)) {
        return E_FAIL;
    }

    const auto receivedSyncrootServerNameMessage = QJsonDocument::fromJson(_localSocket.readAll()).toVariant().toMap();
    const auto serverNameReceived = receivedSyncrootServerNameMessage.value(CfApiShellExtensions::Protocol::ServerNameKey).toString();

    if (serverNameReceived.isEmpty()) {
        disconnectSocketFromServer();
        return E_FAIL;
    }

    // #2 Connect to the current syncroot folder's server
    if (!connectSocketToServer(serverNameReceived)) {
        return E_FAIL;
    }

    // #3 Get a thumbnail format from the current syncroot folder's server and request a thumbnail for a file
    if (!sendMessage(messageRequestThumbnailForFile)) {
        return E_FAIL;
    }

    const auto receivedThumbnailFormatMessage = QJsonDocument::fromJson(_localSocket.readAll()).toVariant().toMap();
    const auto thumbnailFormatReceived = receivedThumbnailFormatMessage.value(CfApiShellExtensions::Protocol::ThumbnailFormatKey).toString();

    if (thumbnailFormatReceived.isEmpty() || thumbnailFormatReceived == CfApiShellExtensions::Protocol::ThumbnailFormatTagEmptyValue) {
        disconnectSocketFromServer();
        return E_FAIL;
    }

    // #4 Notify the current syncroot folder's server that we are ready to receive a thumbnail data (QByteArray)
    const auto readyToAceptThumbnailMessage = QJsonDocument::fromVariant(
        QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestKey,
            QVariantMap{{CfApiShellExtensions::Protocol::ThumbnailProviderRequestAcceptReadyKey, true}}}}).toJson(QJsonDocument::Compact);

    if (!sendMessage(readyToAceptThumbnailMessage)) {
        return E_FAIL;
    }

    // #5 Read the thumbnail data from the current syncroot folder's server
    const auto bitmapReceived = _localSocket.readAll();
    if (bitmapReceived.isEmpty()) {
        disconnectSocketFromServer();
        return E_FAIL;
    }
    const auto imageFromData = QImage::fromData(bitmapReceived, thumbnailFormatReceived.toStdString().c_str()).scaled(QSize(cx, cx));
    if (imageFromData.isNull()) {
        disconnectSocketFromServer();
        return E_FAIL;
    }
    *bitmap = qt_imageToWinHBITMAP(imageFromData);
    disconnectSocketFromServer();
    writeLog("ThumbnailProvider::GetThumbnail success!");
    return S_OK;

    writeLog("ThumbnailProvider::GetThumbnail failure!");
    return E_FAIL;
}