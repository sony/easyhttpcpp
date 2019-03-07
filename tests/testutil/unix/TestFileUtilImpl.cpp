/*
 * Copyright 2017 Sony Corporation
 */
#include "TestFileUtil.h"
#include "TestFileUtilImpl.h"

namespace easyhttpcpp {
namespace testutil {

void TestFileUtilImpl::changeAccessPermission(const std::string& absolutePath, unsigned int mode)
{
	chmod(absolutePath.c_str(), mode);
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
