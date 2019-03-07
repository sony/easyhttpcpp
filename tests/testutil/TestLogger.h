/*
 * Copyright 2018 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTLOGGER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTLOGGER_H_INCLUDED

#include "gtest/gtest.h"

#include "Poco/SingletonHolder.h"

#include "easyhttpcpp/common/BaseLogger.h"

#define EASYHTTPCPP_TESTLOG_E(tag, pFormat, ...) \
        EASYHTTPCPP_BASE_LOG_E(easyhttpcpp::testutil::TestLogger::getInstance(), tag, pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_TESTLOG_W(tag, pFormat, ...) \
        EASYHTTPCPP_BASE_LOG_W(easyhttpcpp::testutil::TestLogger::getInstance(), tag, pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_TESTLOG_I(tag, pFormat, ...) \
        EASYHTTPCPP_BASE_LOG_I(easyhttpcpp::testutil::TestLogger::getInstance(), tag, pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_TESTLOG_D(tag, pFormat, ...) \
        EASYHTTPCPP_BASE_LOG_D(easyhttpcpp::testutil::TestLogger::getInstance(), tag, pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_TESTLOG_V(tag, pFormat, ...) \
        EASYHTTPCPP_BASE_LOG_V(easyhttpcpp::testutil::TestLogger::getInstance(), tag, pFormat, ##__VA_ARGS__)

#define EASYHTTPCPP_TESTLOG_SETUP_END() \
    do { \
        const testing::TestInfo * pTestInfo = testing::UnitTest::GetInstance()->current_test_info(); \
        EASYHTTPCPP_TESTLOG_I(pTestInfo->test_case_name(), "%s SetUp() end.", pTestInfo->name()); \
    } while (false)
#define EASYHTTPCPP_TESTLOG_TEARDOWN_START() \
    do { \
        const testing::TestInfo * pTestInfo = testing::UnitTest::GetInstance()->current_test_info(); \
        EASYHTTPCPP_TESTLOG_I(pTestInfo->test_case_name(), "%s TearDown() start.", pTestInfo->name()); \
    } while (false)

namespace easyhttpcpp {
namespace testutil {

/**
 * @class TestLogger TestLogger.h "TestLogger.h"
 *
 * A singleton Logger with format style support for Tests.
 *
 * The default logging level is set to easyhttpcpp::common::LogLevelVerbose for both debug and release builds.
 *
 * "EASYHTTPCPP_TEST_" will be appended to tags as prefix.
 */
class TestLogger : public easyhttpcpp::common::BaseLogger {
public:
    static TestLogger* getInstance();

private:
    TestLogger();
    virtual ~TestLogger();

    static bool s_terminated;

    friend class Poco::SingletonHolder<TestLogger>;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTLOGGER_H_INCLUDED */
