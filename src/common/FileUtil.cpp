/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "Poco/Exception.h"
#include "Poco/File.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"

#include "FileUtilImpl.h"

namespace easyhttpcpp {
namespace common {

static const std::string Tag = "FileUtil";

bool FileUtil::createDirsIfAbsent(const Poco::File& dir)
{
    try {
        if (dir.path().empty()) {
            EASYHTTPCPP_LOG_D(Tag, "dir is empty.");
            return false;
        }

        Poco::File dirTobeCreated(convertToAbsolutePathString(dir.path()));
        if (dirTobeCreated.exists()) {
            return true;
        }

        dirTobeCreated.createDirectories();
        return dirTobeCreated.exists();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createDirsIfAbsent(%s) failed with error: %s", dir.path().c_str(), e.message().c_str());

        return false;
    }
}

bool FileUtil::removeDirsIfPresent(const Poco::Path& dirPath)
{
    try {
        if (dirPath.toString().empty()) {
            EASYHTTPCPP_LOG_D(Tag, "dirPath is empty.");
            return false;
        }

        Poco::File dirTobeDeleted(convertToAbsolutePathString(dirPath.toString()));
        if (!dirTobeDeleted.exists()) {
            return true;
        }
        if (!dirTobeDeleted.isDirectory()) {
            return false;
        }

        dirTobeDeleted.remove(true);
        return !dirTobeDeleted.exists();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "removeDirsIfPresent(%s) failed with error: %s",
                dirPath.toString().c_str(), e.message().c_str());

        return false;
    }
}

bool FileUtil::removeFileIfPresent(const Poco::File& file)
{
    try {
        Poco::File fileToBeDeleted(convertToAbsolutePathString(file.path()));
        if (!fileToBeDeleted.exists()) {
            return true;
        }
        if (!fileToBeDeleted.isFile()) {
            return false;
        }

        fileToBeDeleted.remove();
        return !fileToBeDeleted.exists();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "removeFileIfPresent(%s) error: %s", file.path().c_str(), e.message().c_str());

        return false;
    }
}

bool FileUtil::moveFile(const Poco::File& sourceFile, const Poco::File& destinationFile)
{
    Poco::File absoluteSourceFile(convertToAbsolutePathString(sourceFile.path()));
    if (!absoluteSourceFile.exists() || !absoluteSourceFile.isFile()) {
        // sourceFile must be an existing file
        return false;
    }

    Poco::File absoluteDestinationFile(convertToAbsolutePathString(destinationFile.path()));
    if (absoluteDestinationFile.exists()) {
        std::string sourceFileAbsolutePath = absoluteSourceFile.path();
        std::string destinationFileAbsolutePath = absoluteDestinationFile.path();
        if (destinationFileAbsolutePath == sourceFileAbsolutePath) {
            // if destination and source are same, it is not necessary to move
            EASYHTTPCPP_LOG_D(Tag, "Move is not necessary because source file and destination file are same [%s]",
                    destinationFile.path().c_str());
            return true;
        }

        EASYHTTPCPP_LOG_I(Tag, "Overwriting existing file before replacing it.");
        EASYHTTPCPP_LOG_V(Tag, "Overwriting existing file[%s] before replacing it with %s", destinationFile.path().c_str(),
                sourceFile.path().c_str());

        if (!removeFileIfPresent(absoluteDestinationFile)) {
            EASYHTTPCPP_LOG_D(Tag, "Failed to delete existing destination file[%s] while moving from source file[%s]. "
                    "This might cause renameTo operation to fail.",
                    destinationFile.path().c_str(), sourceFile.path().c_str());

            // do not return here, try renameTo once. depending upon the filesystem, renameTo might succeed
        }
    }

    try {
        absoluteSourceFile.renameTo(absoluteDestinationFile.path());

        return true;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "moveFile(source[%s], destination[%s]) error: %s",
                sourceFile.path().c_str(), destinationFile.path().c_str(), e.message().c_str());

        return false;
    }
}

std::string FileUtil::convertToAbsolutePathString(const std::string& path, bool extendedPrefix)
{
    return FileUtilImpl::convertToAbsolutePathString(path, extendedPrefix);
}

} /* namespace common */
} /* namespace easyhttpcpp */
