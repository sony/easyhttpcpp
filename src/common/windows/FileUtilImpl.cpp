/*
 * Copyright 2019 Sony Corporation
 */

#include "Poco/Path.h"

#include "FileUtilImpl.h"

namespace easyhttpcpp {
namespace common {

namespace {

const std::string ExtendedPathPrefix = "\\\\?\\";

} /* namespace */

std::string FileUtilImpl::convertToAbsolutePathString(const std::string& path, bool extendedPrefix)
{
    // Windows returns absolute path with extended prefix. See Windows long path info:
    // https://docs.microsoft.com/ja-jp/windows/win32/fileio/naming-a-file#maximum-path-length-limitation
    if (extendedPrefix) {
        return ExtendedPathPrefix + Poco::Path(path).absolute().toString();
    } else {
        return Poco::Path(path).absolute().toString();
    }
}

} /* namespace common */
} /* namespace easyhttpcpp */
