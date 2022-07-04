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

#pragma once
#include "config.h"
#include <shlwapi.h>
#include <thumbcache.h>
#include <unknwn.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.provider.h>

class __declspec(uuid(CFAPI_SHELLEXT_THUMBNAIL_HANDLER_CLASS_ID)) ThumbnailProvider
    : public winrt::implements<ThumbnailProvider, IInitializeWithItem, IThumbnailProvider>
{
public:
    IFACEMETHODIMP Initialize(_In_ IShellItem* item, _In_ DWORD mode);

    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

private:
    winrt::com_ptr<IShellItem2> _itemDest;
    winrt::com_ptr<IShellItem2> _itemSrc;
};