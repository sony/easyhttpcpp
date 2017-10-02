/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/db/SqlException.h"
#include "ExceptionTestUtil.h"

namespace easyhttpcpp {
namespace db {
namespace test {

using easyhttpcpp::common::CoreException;

static const unsigned int SqlIllegalArgumentExceptionExpectedCode = 100200;
static const unsigned int SqlIllegalStateExceptionExpectedCode = 100201;
static const unsigned int SqlExecutionExceptionExpectedCode = 100202;

EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, SqlException, SqlExecutionException, SqlExecutionExceptionExpectedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, SqlException, SqlIllegalArgumentException,
        SqlIllegalArgumentExceptionExpectedCode)
EASYHTTPCPP_EXCEPTION_UNIT_TEST(CoreException, SqlException, SqlIllegalStateException, SqlIllegalStateExceptionExpectedCode)

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
