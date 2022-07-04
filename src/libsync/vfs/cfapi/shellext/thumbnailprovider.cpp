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

        std::wstring sourceItem = L"D:\\work\\cloud-sample\\server\\bird-g272205618_640.jpg";


        IShellItem *pShellItem = 0;

        throw_if_fail((SHCreateItemFromParsingName(
            sourceItem.data(), NULL, __uuidof(_itemSrc), reinterpret_cast<void **>(&_itemSrc))));

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
IFACEMETHODIMP ThumbnailProvider::GetThumbnail(_In_ UINT width, _Out_ HBITMAP *bitmap, _Out_ WTS_ALPHATYPE *alphaType)
{
    // MessageBox(NULL, L"Attach to DLL", L"Attach Now", MB_OK);
    // Open a handle to the file
    writeLog("ThumbnailProvider::GetThumbnail...");
    // Retrieve thumbnails of the placeholders on demand by delegating to the thumbnail of the source items.
    *bitmap = nullptr;
    *alphaType = WTSAT_UNKNOWN;

    try {
        IThumbnailProviderPtr thumbnailProviderSource;
        throw_if_fail(_itemSrc->BindToHandler(NULL, BHID_ThumbnailHandler, __uuidof(thumbnailProviderSource),
            reinterpret_cast<void **>(&thumbnailProviderSource)));
        throw_if_fail(thumbnailProviderSource->GetThumbnail(width, bitmap, alphaType));
    } catch (_com_error exc) {
        std::wstring_convert<convert_type, wchar_t> converter;
        std::string converted_str = converter.to_bytes(std::wstring(exc.ErrorMessage()));
        writeLog(std::string("Error: ") + std::to_string(exc.Error()) + std::string(" ") + converted_str);
    }

    writeLog("ThumbnailProvider::GetThumbnail success!");
    return S_OK;
}