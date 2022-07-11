#pragma once
#include "config.h"

#include <QString>

namespace CfApiShellExtensions {
static constexpr auto ThumbnailProviderServerNamespace = "cfApiShellExtensionsServer";
static const QString ThumbnailProviderMainServerName = APPLICATION_NAME + QStringLiteral(":") + CfApiShellExtensions::ThumbnailProviderServerNamespace;

namespace Protocol {
    static constexpr auto ThumbnailProviderRequestKey = "thumbnailProviderRequest";
    static constexpr auto ThumbnailProviderRequestFilePathKey = "filePath";
    static constexpr auto ThumbnailProviderRequestFileSizeKey = "size";
    static constexpr auto ThumbnailProviderServerNameKey = "serverName";
};
}
