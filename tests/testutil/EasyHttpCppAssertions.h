/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_EASYHTTPCPPASSERTIONS_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_EASYHTTPCPPASSERTIONS_H_INCLUDED

#include "gtest/gtest.h"

#include "Poco/RegularExpression.h"

#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace testutil {

#define EASYHTTPCPP_IS_INSTANCE_OF(pObj, type) (!((pObj).cast<type>().isNull()))

#define EASYHTTPCPP_EXPECT_THROW(statement, expectedException, expectedCode)                                            \
    try {                                                                                                       \
        statement;                                                                                              \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it does not throw a exception.";                                                      \
        ADD_FAILURE() << msg;                                                                                   \
    } catch (const expectedException& e) {                                                                      \
        std::string msg = "failure has occurred with the exception that the " #statement " was throw";          \
        EXPECT_EQ(expectedCode, e.getCode()) << msg;                                                            \
        try {                                                                                                   \
            std::string pattern = easyhttpcpp::common::StringUtil::format("^EASYHTTPCPP-ERR-%u: .*", expectedCode);    \
            Poco::RegularExpression re(pattern);                                                                \
            EXPECT_TRUE(re.match(e.getMessage())) << msg;                                                       \
            EXPECT_TRUE(re.match(e.what())) << msg;                                                             \
        } catch (...) {                                                                                         \
            ADD_FAILURE() << "exception occurred at RegularExpression#match";                                   \
        }                                                                                                       \
        EXPECT_TRUE(e.getCause().isNull()) << msg;                                                              \
    } catch (...) {                                                                                             \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it throws a different type.";                                                         \
        ADD_FAILURE() << msg;                                                                                   \
    }

#define EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(statement, expectedException, expectedCode)                                 \
    try {                                                                                                       \
        statement;                                                                                              \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it does not throw a exception.";                                                      \
        ADD_FAILURE() << msg;                                                                                   \
    } catch (const expectedException& e) {                                                                      \
        std::string msg = "failure has occurred with the exception that the " #statement " was throw";          \
        EXPECT_EQ(expectedCode, e.getCode()) << msg;                                                            \
        try {                                                                                                   \
            std::string pattern = easyhttpcpp::common::StringUtil::format("^EASYHTTPCPP-ERR-%u: .*", expectedCode);    \
            Poco::RegularExpression re(pattern);                                                                \
            EXPECT_TRUE(re.match(e.getMessage())) << msg;                                                       \
            EXPECT_TRUE(re.match(e.what())) << msg;                                                             \
        } catch (...) {                                                                                         \
            ADD_FAILURE() << "exception occurred at RegularExpression#match";                                   \
        }                                                                                                       \
        EXPECT_FALSE(e.getCause().isNull()) << msg;                                                             \
    } catch (...) {                                                                                             \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it throws a different type.";                                                         \
        ADD_FAILURE() << msg;                                                                                   \
    }

#define EASYHTTPCPP_ASSERT_THROW(statement, expectedException, expectedCode)                                            \
    try {                                                                                                       \
        statement;                                                                                              \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it does not throw a exception.";                                                      \
        GTEST_FAIL() << msg;                                                                                    \
    } catch (const expectedException& e) {                                                                      \
        std::string msg = "failure has occurred with the exception that the " #statement " was throw";          \
        ASSERT_EQ(expectedCode, e.getCode()) << msg;                                                            \
        try {                                                                                                   \
            std::string pattern = easyhttpcpp::common::StringUtil::format("^EASYHTTPCPP-ERR-%u: .*", expectedCode);    \
            Poco::RegularExpression re(pattern);                                                                \
            ASSERT_TRUE(re.match(e.getMessage())) << msg;                                                       \
            ASSERT_TRUE(re.match(e.what())) << msg;                                                             \
        } catch (...) {                                                                                         \
            GTEST_FAIL() << "exception occurred at RegularExpression#match";                                    \
        }                                                                                                       \
        ASSERT_TRUE(e.getCause().isNull()) << msg;                                                              \
    } catch (...) {                                                                                             \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it throws a different type.";                                                         \
        GTEST_FAIL() << msg;                                                                                    \
    }

#define EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(statement, expectedException, expectedCode)                                 \
    try {                                                                                                       \
        statement;                                                                                              \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it does not throw a exception.";                                                      \
        GTEST_FAIL() << msg;                                                                                    \
    } catch (const expectedException& e) {                                                                      \
        std::string msg = "failure has occurred with the exception that the " #statement " was throw";          \
        ASSERT_EQ(expectedCode, e.getCode()) << msg;                                                            \
        try {                                                                                                   \
            std::string pattern = easyhttpcpp::common::StringUtil::format("^EASYHTTPCPP-ERR-%u: .*", expectedCode);    \
            Poco::RegularExpression re(pattern);                                                                \
            ASSERT_TRUE(re.match(e.getMessage())) << msg;                                                       \
            ASSERT_TRUE(re.match(e.what())) << msg;                                                             \
        } catch (...) {                                                                                         \
            GTEST_FAIL() << "exception occurred at RegularExpression#match";                                    \
        }                                                                                                       \
        ASSERT_FALSE(e.getCause().isNull()) << msg;                                                             \
    } catch (...) {                                                                                             \
        std::string msg = "Expected: " #statement " throws an exception of type " #expectedException            \
            ".\n  Actual: it throws a different type.";                                                         \
        GTEST_FAIL() << msg;                                                                                    \
    }

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_EASYHTTPCPPASSERTIONS_H_INCLUDED */
