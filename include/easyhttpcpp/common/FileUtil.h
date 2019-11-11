/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_FILEUTIL_H_INCLUDED
#define EASYHTTPCPP_COMMON_FILEUTIL_H_INCLUDED

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API FileUtil {
public:
    static bool createDirsIfAbsent(const Poco::File& dir);
    static bool removeDirsIfPresent(const Poco::Path& dirPath);
    static bool removeFileIfPresent(const Poco::File& file);
    static bool moveFile(const Poco::File& sourceFile, const Poco::File& destinationFile);
    static std::string convertToAbsolutePathString(const std::string& path, bool extendedPrefix = false);

private:
    FileUtil();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_FILEUTIL_H_INCLUDED */
