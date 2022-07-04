// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ShellServices.h"
#include "ClassFactory.h"
#include "thumbnailprovider.h"
#include "contextmenus.h"
#include "customstateprovider.h"

#include "common/utility.h"

#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QEventLoop>

//===============================================================
// ShellServices
//
//    Registers a bunchof COM objects that implement the various
//    whizbangs and gizmos that Shell needs for things like
//    thumbnails, context menus, and custom states.
//
// Fakery Factor:
//
//    Not a lot here. The classes referenced are all fakes,
//    but you could prolly modify them with ease.
//
//===============================================================

namespace
{
    template<typename T>
    DWORD make_and_register_class_object()
    {
        DWORD cookie;
        auto factory = winrt::make<ClassFactory<T>>();
        winrt::check_hresult(CoRegisterClassObject(__uuidof(T), factory.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));
        return cookie;
    }
}

ShellServices *ShellServices::instance()
{
    static ShellServices instance;
    return &instance;
}

void ShellServices::startShellServices()
{
    if (_isRunning) {
        return;
    }

   // registerShellServices();

    auto task = std::thread([this]()
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);

        make_and_register_class_object<ThumbnailProvider>();
        make_and_register_class_object<TestExplorerCommandHandler>();
        make_and_register_class_object<winrt::CfApiShellExtensions::implementation::CustomStateProvider>();

        QEventLoop loop;
        QObject::connect(this, &ShellServices::stop, &loop, [&]() {
            //unregisterShellServices();
            _isRunning = false;
            loop.quit();
        });
        loop.exec();
    });
    task.detach();

    _isRunning = true;
}

bool ShellServices::registerShellServices()
{
    const QList < QPair<QString, QString>> listExtensions = {
        { CFAPI_SHELLEXT_THUMBNAIL_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_THUMBNAIL_HANDLER_CLASS_ID) },
        { CFAPI_SHELLEXT_CUSTOM_STATE_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_CUSTOM_STATE_HANDLER_CLASS_ID) },
        { CFAPI_SHELLEXT_COMMAND_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_COMMAND_HANDLER_CLASS_ID) }
    };
    
    const auto appExePath = QCoreApplication::applicationFilePath();

    for (const auto extension : listExtensions) {
        const QString clsidPath = QString() % R"(Software\Classes\CLSID\)" % extension.second;
        const QString clsidServerPath = QString() % R"(Software\Classes\CLSID\)" % extension.second % R"(\LocalServer32)";
        if (!OCC::Utility::registrySetKeyValue(HKEY_CURRENT_USER, clsidPath, {}, REG_SZ, extension.first)) {
            return false;
        }
        if (!OCC::Utility::registrySetKeyValue(HKEY_CURRENT_USER, clsidServerPath, {}, REG_SZ, appExePath)) {
            return false;
        }
    }

    return true;
}

bool ShellServices::unregisterShellServices()
{
    const QList < QPair<QString, QString>> listExtensions = {
        { CFAPI_SHELLEXT_THUMBNAIL_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_THUMBNAIL_HANDLER_CLASS_ID) },
        { CFAPI_SHELLEXT_CUSTOM_STATE_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_CUSTOM_STATE_HANDLER_CLASS_ID) },
        { CFAPI_SHELLEXT_COMMAND_HANDLER_DISPLAY_NAME, QStringLiteral("{%1}").arg(CFAPI_SHELLEXT_COMMAND_HANDLER_CLASS_ID) }
    };

    for (const auto extension : listExtensions) {
        const QString clsidPath = QString() % R"(Software\Classes\CLSID\)" % extension.second;
        OCC::Utility::registryDeleteKeyTree(HKEY_CURRENT_USER, clsidPath);
    }

    return true;
}
