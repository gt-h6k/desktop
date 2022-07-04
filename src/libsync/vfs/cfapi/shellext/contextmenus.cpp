// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "contextmenus.h"
#include <winrt\Windows.Storage.Provider.h>
#include <winrt\Windows.Foundation.h>
#include <winrt\windows.foundation.collections.h>
#include <shlwapi.h>

namespace winrt {
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Provider;
}

IFACEMETHODIMP TestExplorerCommandHandler::GetTitle(
    _In_opt_ IShellItemArray *items, _Outptr_result_nullonfailure_ PWSTR *name)
{
    *name = nullptr;
    return SHStrDup(L"TestCommand", name);
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
    ULONG cRef = InterlockedDecrement(&_referenceCount);
    if (!cRef) {
        delete this;
    }
    return cRef;
}

