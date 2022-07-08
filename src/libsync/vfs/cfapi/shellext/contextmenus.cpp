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

#include "contextmenus.h"
#include <shlwapi.h>

TestExplorerCommandHandler::TestExplorerCommandHandler()
    : _referenceCount(1)
{
}

IFACEMETHODIMP TestExplorerCommandHandler::GetTitle(_In_opt_ IShellItemArray *items, _Outptr_result_nullonfailure_ PWSTR *name)
{
    *name = nullptr;
    return SHStrDup(L"NcTestCommand", name);
}

IFACEMETHODIMP TestExplorerCommandHandler::GetState(_In_opt_ IShellItemArray *, _In_ BOOL, _Out_ EXPCMDSTATE *cmdState)
{
    *cmdState = ECS_ENABLED;
    return S_OK;
}

IFACEMETHODIMP TestExplorerCommandHandler::GetFlags(_Out_ EXPCMDFLAGS *flags)
{
    *flags = ECF_DEFAULT;
    return S_OK;
}

IFACEMETHODIMP TestExplorerCommandHandler::Invoke(_In_opt_ IShellItemArray *selection, _In_opt_ IBindCtx *)
{
    return S_OK;
}

IFACEMETHODIMP TestExplorerCommandHandler::SetSite(_In_opt_ IUnknown *site)
{
    _site.copy_from(site);
    return S_OK;
}
IFACEMETHODIMP TestExplorerCommandHandler::GetSite(_In_ REFIID riid, _COM_Outptr_ void **site)
{
    return _site->QueryInterface(riid, site);
}

IFACEMETHODIMP TestExplorerCommandHandler::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(TestExplorerCommandHandler, IExplorerCommand),
        QITABENT(TestExplorerCommandHandler, IObjectWithSite),
        {0},
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) TestExplorerCommandHandler::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP_(ULONG) TestExplorerCommandHandler::Release()
{
    const auto refCount = InterlockedDecrement(&_referenceCount);
    if (refCount == 0) {
        delete this;
    }
    return refCount;
}
