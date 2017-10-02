/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/HttpException.h"
#include "ExceptionTestUtil.h"

namespace easyhttpcpp {
namespace test {

using easyhttpcpp::common::CoreException;

static const unsigned int HttpIllegalArgumentExceptedtedCode = 100700;
static const unsigned int HttpIllegalStateExceptionExpectedCode = 100701;
static const unsigned int HttpExecutionExceptionExpectedCode = 100702;
static const unsigned int HttpTimeoutExceptionExpectedCode = 100703;
static const unsigned int HttpSslExceptionExpectedCode = 100704;

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, HttpException, HttpIllegalArgumentException, HttpIllegalArgumentExceptedtedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, HttpException, HttpIllegalStateException,
        HttpIllegalStateExceptionExpectedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, HttpException, HttpExecutionException, HttpExecutionExceptionExpectedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, HttpException, HttpTimeoutException, HttpTimeoutExceptionExpectedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, HttpException, HttpSslException, HttpSslExceptionExpectedCode)

} /* namespace test */
} /* namespace easyhttpcpp */
