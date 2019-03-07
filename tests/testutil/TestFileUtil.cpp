/*
 * Copyright 2017 Sony Corporation
 */
#include "easyhttpcpp/common/CoreLogger.h"
#include "TestFileUtil.h"
#ifdef _WIN32
#include "windows/TestFileUtilImpl.h"
#else
#include "unix/TestFileUtilImpl.h"
#endif

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestFileUtil";

void TestFileUtil::changeAccessPermission(const Poco::Path& absolutePath, unsigned int mode)
{
    TestFileUtilImpl::changeAccessPermission(absolutePath.toString(), mode);
}

void TestFileUtil::setReadOnly(const Poco::Path& path)
{
    changeAccessPermission(path.absolute(), EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY);
}

void TestFileUtil::setFullAccess(const Poco::Path& path)
{
     changeAccessPermission(path.absolute(), EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS);
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
