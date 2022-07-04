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

#include <Shlguid.h>
#include <locale>
#include <codecvt>

IFACEMETHODIMP ThumbnailProvider::Initialize(_In_ IShellItem *item, _In_ DWORD mode)
{
    // MessageBox(NULL, L"Attach to DLL", L"Attach Now", MB_OK);

    try {
        winrt::check_hresult((item->QueryInterface(__uuidof(_itemDest), reinterpret_cast<void **>(&_itemDest))));

        LPWSTR pszName = NULL;
        winrt::check_hresult(_itemDest->GetDisplayName(SIGDN_FILESYSPATH, &pszName));

        std::wstring sourceItem = L"D:\\work\\cloud-sample\\server\\bird-g272205618_640.jpg";

        IShellItem *pShellItem = 0;

        winrt::check_hresult((SHCreateItemFromParsingName(
            sourceItem.data(), NULL, __uuidof(_itemSrc), reinterpret_cast<void **>(&_itemSrc))));
    } catch (...) {
        return winrt::to_hresult();
    }

    return S_OK;
}

// IThumbnailProvider
IFACEMETHODIMP ThumbnailProvider::GetThumbnail(_In_ UINT width, _Out_ HBITMAP *bitmap, _Out_ WTS_ALPHATYPE *alphaType)
{
    // MessageBox(NULL, L"Attach to DLL", L"Attach Now", MB_OK);
    // Open a handle to the file
    // Retrieve thumbnails of the placeholders on demand by delegating to the thumbnail of the source items.
    *bitmap = nullptr;
    *alphaType = WTSAT_UNKNOWN;

    try {
        winrt::com_ptr<IThumbnailProvider> thumbnailProviderSource;
        winrt::check_hresult(_itemSrc->BindToHandler(
            nullptr, BHID_ThumbnailHandler, __uuidof(thumbnailProviderSource), thumbnailProviderSource.put_void()));
        winrt::check_hresult(thumbnailProviderSource->GetThumbnail(width, bitmap, alphaType));
    } catch (...) {
        return winrt::to_hresult();
    }

    return S_OK;
}