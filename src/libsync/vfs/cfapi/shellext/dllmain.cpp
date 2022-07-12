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

#include "cfapishellintegrationclassfactory.h"
#include "vfsexplorercommandhanler.h"
#include "customstateprovider.h"
#include <comdef.h>

long dllReferenceCount = 0;

HINSTANCE instanceHandle = NULL;

HRESULT CustomStateProvider_CreateInstance(REFIID riid, void **ppv);
HRESULT TestExplorerCommandHandler_CreateInstance(REFIID riid, void **ppv);

const ClassObjectInit listClassesSupported[] = {
    {&__uuidof(winrt::CfApiShellExtensions::implementation::CustomStateProvider), CustomStateProvider_CreateInstance},
    {&__uuidof(VfsExplorerCommandHandler), TestExplorerCommandHandler_CreateInstance}
};

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        instanceHandle = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }

    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    return dllReferenceCount == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    return CfApiShellIntegrationClassFactory::CreateInstance(clsid, listClassesSupported, ARRAYSIZE(listClassesSupported), riid, ppv);
}

HRESULT CustomStateProvider_CreateInstance(REFIID riid, void **ppv)
{
    DWORD cookie;
    try {
        auto customStateProviderInstance = winrt::make_self<winrt::CfApiShellExtensions::implementation::CustomStateProvider>();
        return customStateProviderInstance->QueryInterface(riid, ppv);
    } 
    catch (_com_error exc) {
        return exc.Error();
    }
}

HRESULT TestExplorerCommandHandler_CreateInstance(REFIID riid, void **ppv)
{
    auto *testExplorerCommandHandler = new (std::nothrow) VfsExplorerCommandHandler();
    if (!testExplorerCommandHandler) {
        return E_OUTOFMEMORY;
    }
    const auto hresult = testExplorerCommandHandler->QueryInterface(riid, ppv);
    testExplorerCommandHandler->Release();
    return hresult;
}
