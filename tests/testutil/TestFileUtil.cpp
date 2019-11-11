/*
 * Copyright 2017 Sony Corporation
 */
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "TestConstants.h"
#include "TestFileUtil.h"
#ifdef _WIN32
#include "windows/TestFileUtilImpl.h"
#else
#include "unix/TestFileUtilImpl.h"
#endif

using easyhttpcpp::common::FileUtil;

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestFileUtil";

void TestFileUtil::changeAccessPermission(const Poco::Path& absolutePath, unsigned int mode)
{
    TestFileUtilImpl::changeAccessPermission(
            FileUtil::convertToAbsolutePathString(absolutePath.toString(), true), mode);
}

void TestFileUtil::setReadOnly(const Poco::Path& path)
{
    changeAccessPermission(path.absolute(), EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY);
}

void TestFileUtil::setFullAccess(const Poco::Path& path)
{
     changeAccessPermission(path.absolute(), EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS);
}

void TestFileUtil::appendLongPathDir(Poco::Path& path)
{
    // path/<dir name 64byte>/<dir name 64byte>/<dir name 64byte>/<dir name 64byte>/
    path.append(Poco::Path(TestConstants::LongDirName64byte));
    path.append(Poco::Path(TestConstants::LongDirName64byte));
    path.append(Poco::Path(TestConstants::LongDirName64byte));
    path.append(Poco::Path(TestConstants::LongDirName64byte));
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
