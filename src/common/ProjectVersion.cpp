/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/ProjectVersion.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {

ProjectVersion::ProjectVersion()
{
}

std::string ProjectVersion::getMajor()
{
    return EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MAJOR);
}

std::string ProjectVersion::getMinor()
{
    return EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MINOR);
}

std::string ProjectVersion::getPatch()
{
    return EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_PATCH);
}

std::string ProjectVersion::getExtension()
{
    return EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_EXT);
}

std::string ProjectVersion::asString()
{
    return StringUtil::formatVersion(getMajor(), getMinor(), getPatch(), getExtension());
}

} /* namespace common */
} /* namespace easyhttpcpp */
