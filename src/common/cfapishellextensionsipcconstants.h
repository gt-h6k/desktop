#pragma once
#include "config.h"

#include <QString>
#include <QByteArray>

namespace CfApiShellExtensions {
static constexpr auto IpcServerNamespace = "cfApiShellExtensionsServer";
static const QString IpcMainServerName = APPLICATION_NAME + QStringLiteral(":") + CfApiShellExtensions::IpcServerNamespace;

namespace Protocol {
    static constexpr auto ThumbnailProviderRequestKey = "thumbnailProviderRequest";
    static constexpr auto ThumbnailProviderRequestAcceptReadyKey = "thumbnailAccepReady";
    static constexpr auto ThumbnailProviderRequestFilePathKey = "filePath";
    static constexpr auto ThumbnailProviderRequestFileSizeKey = "size";
    static constexpr auto ThumbnailFormatKey = "format";
    static constexpr auto ThumbnailFormatTagEmptyValue = "empty";
    static const QByteArray ThumbnailFormatEmptyMessage = QString("{\"%1\":\"%2\"}").arg(ThumbnailFormatKey).arg(ThumbnailFormatTagEmptyValue).toUtf8();
    static constexpr auto ServerNameKey = "serverName";
};
}