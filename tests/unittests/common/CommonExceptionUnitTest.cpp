/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/CommonException.h"
#include "ExceptionTestUtil.h"

namespace easyhttpcpp {
namespace common {
namespace test {

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, PocoException, 100000);
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, StdException, 100001);
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, FutureIllegalStateException, 100002);
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, FutureCancellationException, 100003);
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, FutureExecutionException, 100004);
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, CommonException, FutureTimeoutException, 100005);

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
