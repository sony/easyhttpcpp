/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/messagedigest/MessageDigestException.h"
#include "ExceptionTestUtil.h"

using easyhttpcpp::common::CoreException;

namespace easyhttpcpp {
namespace messagedigest {
namespace test {

namespace {
static const unsigned int MessageDigestIllegalArgumentExceptionExpectedCode = 100300;
static const unsigned int MessageDigestIllegalStateExceptionExpectedCode = 100301;
static const unsigned int MessageDigestExecutionExceptionExpectedCode = 100302;
}

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, MessageDigestException, MessageDigestIllegalArgumentException, MessageDigestIllegalArgumentExceptionExpectedCode)

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, MessageDigestException, MessageDigestIllegalStateException, MessageDigestIllegalStateExceptionExpectedCode)

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, MessageDigestException, MessageDigestExecutionException, MessageDigestExecutionExceptionExpectedCode)

} /* namespace test */
} /* namespace messagedigest */
} /* namespace easyhttpcpp */
