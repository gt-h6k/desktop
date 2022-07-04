// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <QObject>

class ShellServices : public QObject
{
    Q_OBJECT

    ShellServices() = default;

public:
    static ShellServices *instance();
    ~ShellServices() = default;

    void startShellServices();

signals:
    void stop();

private:
    static bool registerShellServices();
    static bool unregisterShellServices();

private:
    bool _isRunning = false;
};

