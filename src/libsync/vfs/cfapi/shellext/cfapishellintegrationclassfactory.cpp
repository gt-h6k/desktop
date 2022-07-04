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

#include <new>

#include "logger.h"

extern long dllReferenceCount;

HRESULT CfApiShellIntegrationClassFactory::CreateInstance(REFCLSID clsid, const CLASS_OBJECT_INIT *pClassObjectInits, size_t cClassObjectInits, REFIID riid, void **ppv)
{
    char clsidGuid[40] = {0};
    sprintf(clsidGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", clsid.Data1, clsid.Data2, clsid.Data3,
        clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3], clsid.Data4[4], clsid.Data4[5], clsid.Data4[6],
        clsid.Data4[7]);

    char riidGuid[40] = {0};
    sprintf(riidGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1, riid.Data2, riid.Data3,
        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6],
        riid.Data4[7]);

    // Write data to the file
    std::string strText = "CfApiShellIntegrationClassFactory::CreateInstance 1 clsidGuid: " + std::string(clsidGuid) + std::string(" riidGuid: ")
        + std::string(riidGuid) + std::string("\n"); // For C use LPSTR (char*) or LPWSTR (wchar_t*)
    writeLog(strText);
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    for (size_t i = 0; i < cClassObjectInits; i++) {
        if (clsid == *pClassObjectInits[i].pClsid) {
            IClassFactory *pClassFactory = new (std::nothrow) CfApiShellIntegrationClassFactory(pClassObjectInits[i].pfnCreate);
            hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr)) {
                strText = "CfApiShellIntegrationClassFactory::CreateInstance 2 clsidGuid: " + std::string(clsidGuid)
                    + std::string(" riidGuid: ") + std::string(riidGuid)
                    + std::string("\n"); // For C use LPSTR (char*) or LPWSTR (wchar_t*)
                writeLog(strText);
                hr = pClassFactory->QueryInterface(riid, ppv);
                pClassFactory->Release();
            }
            break; // match found
        }
    }
    strText = "CfApiShellIntegrationClassFactory::CreateInstance 3 clsidGuid: " + std::string(clsidGuid)
        + std::string(" riidGuid: ") + std::string(riidGuid)
        + std::string("\n"); // For C use LPSTR (char*) or LPWSTR (wchar_t*)
    writeLog(strText);
    return hr;
}

// IUnknown
IFACEMETHODIMP CfApiShellIntegrationClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hResult = S_OK;

    char riidGuid[40] = {0};
    sprintf(riidGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1, riid.Data2, riid.Data3,
        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6],
        riid.Data4[7]);
    std::string strText = "CfApiShellIntegrationClassFactory::QueryInterface riidGuid: " + std::string(riidGuid) + std::string("\n");
    writeLog(strText);

    if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IClassFactory, riid)) {
        *ppv = static_cast<IUnknown *>(this);
        strText =
            "CfApiShellIntegrationClassFactory::QueryInterface 2 riidGuid: " + std::string(riidGuid) + std::string("\n");
        writeLog(strText);
        AddRef();
    } else {
        strText =  "CfApiShellIntegrationClassFactory::QueryInterface 3 riidGuid: " + std::string(riidGuid) + std::string("\n");
        writeLog(strText);
        hResult = E_NOINTERFACE;
        *ppv = nullptr;
    }

    return hResult;
}

IFACEMETHODIMP_(ULONG) CfApiShellIntegrationClassFactory::AddRef()
{
    std::string strText = "CfApiShellIntegrationClassFactory::AddRef\n";
    writeLog(strText);
    return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP_(ULONG) CfApiShellIntegrationClassFactory::Release()
{
    std::string strText = "CfApiShellIntegrationClassFactory::Release\n";
    writeLog(strText);
    long cRef = InterlockedDecrement(&_referenceCount);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

// IClassFactory
IFACEMETHODIMP CfApiShellIntegrationClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    char riidGuid[40] = {0};
    sprintf(riidGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1, riid.Data2, riid.Data3,
        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6],
        riid.Data4[7]);
    std::string strText =
        "CfApiShellIntegrationClassFactory::CreateInstance 1 riidGuid: " + std::string(riidGuid) + std::string("\n");
    writeLog(strText);
    if (punkOuter) {
        strText = "CfApiShellIntegrationClassFactory::CreateInstance 2 riidGuid: " + std::string(riidGuid)
            + std::string("\n");
        writeLog(strText);
        return CLASS_E_NOAGGREGATION;
    }
    strText =  "CfApiShellIntegrationClassFactory::CreateInstance 3 riidGuid: " + std::string(riidGuid) + std::string("\n");
    writeLog(strText);
    return _pfnCreate(riid, ppv);
}

IFACEMETHODIMP CfApiShellIntegrationClassFactory::LockServer(BOOL fLock)
{
    std::string strText = "LockServer 1\n";
    writeLog(strText);
    if (fLock) {
        strText = "LockServer 2\n";
        writeLog(strText);
        InterlockedIncrement(&dllReferenceCount);
    } else {
        strText = "LockServer 3\n";
        writeLog(strText);
        InterlockedDecrement(&dllReferenceCount);
    }
    strText = "LockServer 4\n";
    writeLog(strText);
    return S_OK;
}

CfApiShellIntegrationClassFactory::CfApiShellIntegrationClassFactory(PFNCREATEINSTANCE pfnCreate)
    : _referenceCount(1)
    , _pfnCreate(pfnCreate)
{
    std::string strText = "CfApiShellIntegrationClassFactory\n";
    writeLog(strText);
    InterlockedIncrement(&dllReferenceCount);
}

CfApiShellIntegrationClassFactory::~CfApiShellIntegrationClassFactory()
{
    std::string strText = "~CfApiShellIntegrationClassFactory\n";
    writeLog(strText);
    InterlockedDecrement(&dllReferenceCount);
}