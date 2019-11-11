/*
 * Copyright 2019 Sony Corporation
 */

#include "Poco/Path.h"

#include "FileUtilImpl.h"

namespace easyhttpcpp {
namespace common {

std::string FileUtilImpl::convertToAbsolutePathString(const std::string& path, bool extendedPrefix)
{
    // Linux returns absolute path.
    return Poco::Path(path).absolute().toString();
}

} /* namespace common */
} /* namespace easyhttpcpp */
