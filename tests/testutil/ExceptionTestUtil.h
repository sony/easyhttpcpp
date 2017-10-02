/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_EXCEPTIONTESTUTIL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_EXCEPTIONTESTUTIL_H_INCLUDED

#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace testutil {

class ExceptionUnitTestParam {
public:
    const char* m_pMessage;
    std::exception m_cause;
    unsigned int m_code;
    const char* m_pExpectedMessage;
};

static const char* const ExceptionMessagePrefix = "EASYHTTPCPP-ERR-";

////////////////////////////////////////////////////////////
// Macro to simplify the exception test
//  GROUPCLS : parent group exception class for TARGETCLS
//  SUBGROUPCLS : parent subgroup exception class for TARGETCLS
//  TARGETCLS : target class for test
//  CODE : expected code for the exception
//
#define EASYHTTPCPP_EXCEPTION_UNIT_TEST(GROUPCLS, SUBGROUPCLS, TARGETCLS, CODE)                                             \
    class TARGETCLS##UnitTest : public testing::Test {                                                              \
    };                                                                                                              \
    static const easyhttpcpp::testutil::ExceptionUnitTestParam TARGETCLS##_TestParamsWithNoUnderlyingError[] = {   \
        {"Something happened", TARGETCLS("dummy"), CODE, "Something happened"},                                     \
        {" ", TARGETCLS("dummy"), CODE, " "},                                                                       \
        {"", TARGETCLS("dummy"), CODE, "Unknown exception occurred."}                                               \
    };                                                                                                              \
    static const easyhttpcpp::testutil::ExceptionUnitTestParam TARGETCLS##_TestParamsWithUnderlyingError[] = {     \
        {"Something happened", TARGETCLS("easyhttpcpp"), CODE, "Something happened"},                                    \
        {"Something happened", Poco::Exception("poco"), CODE, "Something happened"},                                \
        {"Something happened", std::runtime_error("runtime"), CODE, "Something happened"},                          \
        {" ", TARGETCLS("easyhttpcpp"), CODE, " "},                                                                      \
        {" ", Poco::Exception("poco"), CODE, " "},                                                                  \
        {" ", std::runtime_error("runtime"), CODE, " "},                                                            \
        {"", TARGETCLS("easyhttpcpp"), CODE, "Unknown exception occurred."},                                             \
        {"", Poco::Exception("poco"), CODE, "Unknown exception occurred."},                                         \
        {"", std::runtime_error("runtime"), CODE, "Unknown exception occurred."}                                    \
    };                                                                                                              \
    class TARGETCLS##UnitTestWithNoUnderlyingError :                                                                \
        public ::testing::TestWithParam<easyhttpcpp::testutil::ExceptionUnitTestParam> {                           \
    };                                                                                                              \
    INSTANTIATE_TEST_CASE_P(TARGETCLS##UnitTest, TARGETCLS##UnitTestWithNoUnderlyingError,                          \
        ::testing::ValuesIn(TARGETCLS##_TestParamsWithNoUnderlyingError));                                          \
    TEST_P(TARGETCLS##UnitTestWithNoUnderlyingError, constructor_BehavesPropery_WithNoUnderlyingError)              \
    {                                                                                                               \
        try {                                                                                                       \
            throw TARGETCLS(GetParam().m_pMessage);                                                                 \
        } catch (const TARGETCLS& e) {                                                                              \
            EXPECT_EQ(GetParam().m_code, e.getCode());                                                              \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, GetParam().m_pExpectedMessage);               \
            EXPECT_STREQ(expectedMessage.c_str(), e.getMessage().c_str());                                          \
            EXPECT_STREQ(expectedMessage.c_str(), e.what());                                                        \
            EXPECT_TRUE(e.getCause().isNull());                                                                     \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    class TARGETCLS##UnitTestWithUnderlyingError :                                                                  \
        public ::testing::TestWithParam<easyhttpcpp::testutil::ExceptionUnitTestParam> {                           \
    };                                                                                                              \
    INSTANTIATE_TEST_CASE_P(TARGETCLS##UnitTest, TARGETCLS##UnitTestWithUnderlyingError,                            \
        ::testing::ValuesIn(TARGETCLS##_TestParamsWithUnderlyingError));                                            \
    TEST_P(TARGETCLS##UnitTestWithUnderlyingError, constructor_BehavesPropery_WithUnderlyingError)                  \
    {                                                                                                               \
        try {                                                                                                       \
            throw TARGETCLS(GetParam().m_pMessage, GetParam().m_cause);                                             \
        } catch (const TARGETCLS& e) {                                                                              \
            EXPECT_EQ(GetParam().m_code, e.getCode());                                                              \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, GetParam().m_pExpectedMessage);               \
            EXPECT_STREQ(expectedMessage.c_str(), e.getMessage().c_str());                                          \
            EXPECT_STREQ(expectedMessage.c_str(), e.what());                                                        \
            EXPECT_FALSE(e.getCause().isNull());                                                                    \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, copyConstructor_CopiesAllProperties)                                                \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message1 = "exception";                                                                         \
        try {                                                                                                       \
            throw TARGETCLS(message1, Poco::Exception("Poco"));                                                     \
        } catch (const TARGETCLS& expectedException) {                                                              \
            /* When: Copy exception by copy constructor. */                                                         \
            TARGETCLS actualException(expectedException);                                                           \
            /* Then: Should be the same content. */                                                                 \
            EXPECT_EQ(expectedException.getCode(), actualException.getCode());                                      \
            EXPECT_STREQ(expectedException.getMessage().c_str(), actualException.getMessage().c_str());             \
            EXPECT_STREQ(expectedException.what(), actualException.what());                                         \
            EXPECT_FALSE(actualException.getCause().isNull());                                                      \
            EXPECT_EQ(expectedException.getCause()->getCode(), actualException.getCause()->getCode());              \
            EXPECT_STREQ(expectedException.getCause()->getMessage().c_str(),                                        \
                    actualException.getCause()->getMessage().c_str());                                              \
            EXPECT_STREQ(expectedException.getCause()->what(), actualException.getCause()->what());                 \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, assignmentOperator_CopiesAllProperties)                                             \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message1 = "exception";                                                                         \
        try {                                                                                                       \
            throw TARGETCLS(message1, Poco::Exception("Poco"));                                                     \
        } catch (const TARGETCLS& expectedException) {                                                              \
            /* When: Copy exception by assignment operator. */                                                      \
            TARGETCLS actualException("dummy");                                                                     \
            actualException = expectedException;                                                                    \
            /* Then: Should be the same content. */                                                                 \
            EXPECT_EQ(expectedException.getCode(), actualException.getCode());                                      \
            EXPECT_STREQ(expectedException.getMessage().c_str(), actualException.getMessage().c_str());             \
            EXPECT_STREQ(expectedException.what(), actualException.what());                                         \
            EXPECT_FALSE(actualException.getCause().isNull());                                                      \
            EXPECT_EQ(expectedException.getCause()->getCode(), actualException.getCause()->getCode());              \
            EXPECT_STREQ(expectedException.getCause()->getMessage().c_str(),                                        \
                    actualException.getCause()->getMessage().c_str());                                              \
            EXPECT_STREQ(expectedException.getCause()->what(), actualException.getCause()->what());                 \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, clone_CopiesAllProperties)                                                          \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message1 = "exception";                                                                         \
        try {                                                                                                       \
            throw TARGETCLS(message1, Poco::Exception("Poco"));                                                     \
        } catch (const easyhttpcpp::common::BaseException& expectedException) {                                    \
            /* When: Copy exception by clone exception. */                                                          \
            TARGETCLS::Ptr actualException = expectedException.clone();                                             \
            /* Then: Should be the same content. */                                                                 \
            EXPECT_EQ(expectedException.getCode(), actualException->getCode());                                     \
            EXPECT_STREQ(expectedException.getMessage().c_str(), actualException->getMessage().c_str());            \
            EXPECT_STREQ(expectedException.what(), actualException->what());                                        \
            EXPECT_FALSE(actualException->getCause().isNull());                                                     \
            EXPECT_EQ(expectedException.getCause()->getCode(), actualException->getCause()->getCode());             \
            EXPECT_STREQ(expectedException.getCause()->getMessage().c_str(),                                        \
                    actualException->getCause()->getMessage().c_str());                                             \
            EXPECT_STREQ(expectedException.getCause()->what(), actualException->getCause()->what());                \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, rethrow_Succeeds)                                                                   \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message1 = "exception";                                                                         \
        const TARGETCLS* expectException;                                                                           \
        try {                                                                                                       \
            throw TARGETCLS(message1, Poco::Exception("Poco"));                                                     \
        } catch (const TARGETCLS& e) {                                                                              \
            try {                                                                                                   \
                /* When: rethrow exception. */                                                                      \
                expectException = &e;                                                                               \
                e.rethrow();                                                                                        \
            } catch (const TARGETCLS& actualException) {                                                            \
                /* Then: Should be the same content. */                                                             \
                EXPECT_EQ(expectException->getCode(), actualException.getCode());                                   \
                EXPECT_STREQ(expectException->getMessage().c_str(), actualException.getMessage().c_str());          \
                EXPECT_STREQ(expectException->what(), actualException.what());                                      \
                EXPECT_FALSE(actualException.getCause().isNull());                                                  \
                EXPECT_EQ(expectException->getCause()->getCode(), actualException.getCause()->getCode());           \
                EXPECT_STREQ(expectException->getCause()->getMessage().c_str(),                                     \
                        actualException.getCause()->getMessage().c_str());                                          \
                EXPECT_STREQ(expectException->getCause()->what(), actualException.getCause()->what());              \
            } catch (...) {                                                                                         \
                GTEST_FAIL() << "Failed to catch target exception at second time.";                                 \
            }                                                                                                       \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception at first time.";                                      \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, threwException_Succeeds_WhenSpecifiedGroupExceptionAtCatch)                         \
    {                                                                                                               \
        /* Given: Preparing the exception message */                                                                \
        std::string message = "exception";                                                                          \
        try {                                                                                                       \
            throw TARGETCLS(message);                                                                               \
        /* When: catch group exception. */                                                                          \
        } catch (const GROUPCLS& e) {                                                                               \
            /* Then: Should be the same exception as threw exception. */                                            \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message.c_str());                             \
            EXPECT_EQ(CODE, e.getCode());                                                                           \
            EXPECT_STREQ(expectedMessage.c_str(), e.getMessage().c_str());                                          \
            EXPECT_STREQ(expectedMessage.c_str(), e.what());                                                        \
            EXPECT_TRUE(e.getCause().isNull());                                                                     \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, threwException_Succeeds_WhenSpecifiedSubGroupExceptionAtCatch)                      \
    {                                                                                                               \
        /* Given: Preparing the exception message */                                                                \
        std::string message = "exception";                                                                          \
        try {                                                                                                       \
            throw TARGETCLS(message);                                                                               \
        /* When: catch subgroup exception. */                                                                       \
        } catch (const SUBGROUPCLS& e) {                                                                            \
            /* Then: Should be the same exception as threw exception. */                                            \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message.c_str());                             \
            EXPECT_EQ(CODE, e.getCode());                                                                           \
            EXPECT_STREQ(expectedMessage.c_str(), e.getMessage().c_str());                                          \
            EXPECT_STREQ(expectedMessage.c_str(), e.what());                                                        \
            EXPECT_TRUE(e.getCause().isNull());                                                                     \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, threwException_Succeeds_WhenSpecifiedStdExceptionAtCatch)                           \
    {                                                                                                               \
        /* Given: Preparing the exception message */                                                                \
        std::string message = "exception";                                                                          \
        try {                                                                                                       \
            throw TARGETCLS(message);                                                                               \
        /* When: catch subgroup exception. */                                                                       \
        } catch (const std::exception& e) {                                                                         \
            /* Then: Should be the same exception as threw exception. */                                            \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message.c_str());                             \
            EXPECT_STREQ(expectedMessage.c_str(), e.what());                                                        \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, getCause_Succeeds_WhenNestedCauseByQuiverException)                                 \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message = "easyhttpcpp";                                                                             \
        try {                                                                                                       \
            /* When: Throw exception. */                                                                            \
            throw TARGETCLS("easyhttpcpp", TARGETCLS(message.c_str()));                                                  \
        } catch (const TARGETCLS& e) {                                                                              \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message.c_str());                             \
            easyhttpcpp::common::BaseException::Ptr cause = e.getCause();                                          \
            /* Then: Should the contents of the cause can be acquired. */                                           \
            ASSERT_FALSE(cause.isNull());                                                                           \
            EXPECT_EQ(CODE, cause->getCode());                                                                      \
            EXPECT_STREQ(expectedMessage.c_str(), cause->getMessage().c_str());                                     \
            EXPECT_STREQ(expectedMessage.c_str(), cause->what());                                                   \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, getCause_Succeeds_WhenNestedCauseByPocoException)                                   \
    {                                                                                                               \
        /* Given: Preparing the Poco::Exception. */                                                                 \
        Poco::IOException pocoException("poco");                                                                    \
        try {                                                                                                       \
            /* When: Throw exception. */                                                                            \
            throw TARGETCLS("easyhttpcpp", pocoException);                                                               \
        } catch (const TARGETCLS& e) {                                                                              \
            unsigned int expectedCode = 100000;                                                                     \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s(%d) %s",               \
                easyhttpcpp::testutil::ExceptionMessagePrefix, expectedCode, pocoException.name(),                 \
                pocoException.code(), pocoException.message().c_str());                                             \
            easyhttpcpp::common::BaseException::Ptr cause = e.getCause();                                          \
            /* Then: Should the contents of the cause can be acquired. */                                           \
            ASSERT_FALSE(cause.isNull());                                                                           \
            EXPECT_EQ(expectedCode, cause->getCode());                                                              \
            EXPECT_STREQ(expectedMessage.c_str(), cause->getMessage().c_str());                                     \
            EXPECT_STREQ(expectedMessage.c_str(), cause->what());                                                   \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, getCause_Succeeds_WhenNestedCauseByStdException)                                    \
    {                                                                                                               \
        /* Given: Preparing the std::exception. */                                                                  \
        std::runtime_error stdException("runtime");                                                                 \
        try {                                                                                                       \
            /* When: Throw exception. */                                                                            \
            throw TARGETCLS("easyhttpcpp", stdException);                                                                \
        } catch (const TARGETCLS& e) {                                                                              \
            unsigned int expectedCode = 100001;                                                                     \
            std::string expectedMessage = easyhttpcpp::common::StringUtil::format("%s%u: %s",                      \
                easyhttpcpp::testutil::ExceptionMessagePrefix, expectedCode, stdException.what());                 \
            easyhttpcpp::common::BaseException::Ptr cause = e.getCause();                                          \
            /* Then: Should the contents of the cause can be acquired. */                                           \
            ASSERT_FALSE(cause.isNull());                                                                           \
            EXPECT_EQ(expectedCode, cause->getCode());                                                              \
            EXPECT_STREQ(expectedMessage.c_str(), cause->getMessage().c_str());                                     \
            EXPECT_STREQ(expectedMessage.c_str(), cause->what());                                                   \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception.";                                                    \
        }                                                                                                           \
    }                                                                                                               \
    TEST_F(TARGETCLS##UnitTest, getCause_Succeeds_WhenManyNestedCause)                                              \
    {                                                                                                               \
        /* Given: Preparing the exception message. */                                                               \
        std::string message1 = "exception 1";                                                                       \
        std::string message2 = "exception 2";                                                                       \
        std::string message3 = "exception 3";                                                                       \
        /* When: Throw nest exception. */                                                                           \
        try {                                                                                                       \
            throw TARGETCLS(message3);                                                                              \
        } catch (const TARGETCLS& e) {                                                                              \
            try {                                                                                                   \
                throw TARGETCLS(message2, e);                                                                       \
            } catch (const TARGETCLS& e) {                                                                          \
                try {                                                                                               \
                    throw TARGETCLS(message1, e);                                                                   \
                } catch (const TARGETCLS& e) {                                                                      \
                    /* Then : Nested exception can be acquired. */                                                  \
                    TARGETCLS::Ptr nestedException1 = e.getCause();                                                 \
                    ASSERT_FALSE(nestedException1.isNull());                                                        \
                    TARGETCLS::Ptr nestedException2 = nestedException1->getCause();                                 \
                    ASSERT_FALSE(nestedException2.isNull());                                                        \
                    ASSERT_TRUE(nestedException2->getCause().isNull());                                             \
                    std::string expectedExceptionMessage =                                                          \
                            easyhttpcpp::common::StringUtil::format("%s%u: %s",                                    \
                            easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message1.c_str());                \
                    std::string expectedNestedExceptionMessage1 =                                                   \
                            easyhttpcpp::common::StringUtil::format("%s%u: %s",                                    \
                            easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message2.c_str());                \
                    std::string expectedNestedExceptionMessage2 =                                                   \
                            easyhttpcpp::common::StringUtil::format("%s%u: %s",                                    \
                            easyhttpcpp::testutil::ExceptionMessagePrefix, CODE, message3.c_str());                \
                    EXPECT_STREQ(expectedExceptionMessage.c_str(), e.what());                                       \
                    EXPECT_STREQ(expectedNestedExceptionMessage1.c_str(), nestedException1->what());                \
                    EXPECT_STREQ(expectedNestedExceptionMessage2.c_str(), nestedException2->what());                \
                } catch (...) {                                                                                     \
                    GTEST_FAIL() << "Failed to catch target exception at third time.";                              \
                }                                                                                                   \
            } catch (...) {                                                                                         \
                GTEST_FAIL() << "Failed to catch target exception at second time.";                                 \
            }                                                                                                       \
        } catch (...) {                                                                                             \
            GTEST_FAIL() << "Failed to catch target exception at first time.";                                      \
        }                                                                                                           \
    }

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_EXCEPTIONTESTUTIL_H_INCLUDED */
