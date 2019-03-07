/*
 * Copyright 2017 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"
#include "Poco/Logger.h"
#include "Poco/Message.h"
#include "Poco/RegularExpression.h"

#include "easyhttpcpp/common/CoreLogger.h"

namespace easyhttpcpp {
namespace common {
namespace test {

static const char ExpectedRegexFormat[] = "EASYHTTPCPP_%s LogLv=%c line=[0-9]+ %s";
static const char ConvertToLogLevelChars[] = "VDIWE";
static const Poco::Message::Priority ConvertToPocoMessagePriority[] = {Poco::Message::PRIO_TRACE,
    Poco::Message::PRIO_DEBUG, Poco::Message::PRIO_INFORMATION, Poco::Message::PRIO_WARNING, Poco::Message::PRIO_ERROR};

class CoreLoggerUnitTest : public testing::Test {
protected:

    virtual void SetUp()
    {
        cleanupLogger();
    }

    virtual void TearDown()
    {
        cleanupLogger();
    }

    void cleanupLogger()
    {
        Poco::Logger::shutdown();
        CoreLogger::getInstance()->resetToDefaults();
    }

    class MockPocoChannel : public Poco::Channel {
    public:
        typedef Poco::AutoPtr<MockPocoChannel> Ptr;

        MOCK_METHOD0(getLogWriterName, std::string&());
        MOCK_METHOD0(open, void());
        MOCK_METHOD0(close, void());
        MOCK_METHOD1(log, void(const Poco::Message& msg));
        MOCK_METHOD2(setProperty, void(const std::string& name, const std::string& value));
        MOCK_CONST_METHOD1(getProperty, std::string(const std::string& name));
    };
};

TEST_F(CoreLoggerUnitTest, getInstance_ReturnsSingletonInstance_WhenCalledTwice)
{
    // Given: -

    // When: CoreLogger#getInstance()を呼ぶ
    CoreLogger* pCoreLogger1 = CoreLogger::getInstance();
    CoreLogger* pCoreLogger2 = CoreLogger::getInstance();

    // Then: インスタンスが同じである
    EXPECT_EQ(pCoreLogger1, pCoreLogger2);
}

class EASYHTTPCPPCORELOGXParameterizedTestParam {
public:
    LogLevel m_messageLogLevel;
    LogLevel m_setLoggingLevel;
    std::string m_tag;
    std::string m_message;

    unsigned int m_logCalledTimes;
};

static EASYHTTPCPPCORELOGXParameterizedTestParam EASYHTTPCPPCORELOGXParameterizedTestParams[] = {
    // m_messageLogLevel, m_setLoggingLevel, m_tag, m_message, m_logCalledTimes
    {LogLevelError, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelError, LogLevelError, "Tag_Error", "Something terrible happened.", 1},
    {LogLevelError, LogLevelWarning, "Tag_Error", "Something terrible happened.", 1},
    {LogLevelError, LogLevelInfo, "Tag_Error", "Something terrible happened.", 1},
    {LogLevelError, LogLevelDebug, "Tag_Error", "Something terrible happened.", 1},
    {LogLevelError, LogLevelVerbose, "Tag_Error", "Something terrible happened.", 1},

    {LogLevelWarning, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelWarning, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelWarning, LogLevelWarning, "Tag_Warning", "Internal warning occurred.", 1},
    {LogLevelWarning, LogLevelInfo, "Tag_Warning", "Internal warning occurred.", 1},
    {LogLevelWarning, LogLevelDebug, "Tag_Warning", "Internal warning occurred.", 1},
    {LogLevelWarning, LogLevelVerbose, "Tag_Warning", "Internal warning occurred.", 1},

    {LogLevelInfo, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelInfo, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelInfo, LogLevelWarning, "Tag_Dummy", "dummy message", 0},
    {LogLevelInfo, LogLevelInfo, "Tag_Information", "Information of processing.", 1},
    {LogLevelInfo, LogLevelDebug, "Tag_Information", "Information of processing.", 1},
    {LogLevelInfo, LogLevelVerbose, "Tag_Information", "Information of processing.", 1},

#ifndef NDEBUG
    {LogLevelDebug, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelWarning, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelInfo, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelDebug, "Tag_Debug", "Information to help of debug.", 1},
    {LogLevelDebug, LogLevelVerbose, "Tag_Debug", "Information to help of debug.", 1},

    {LogLevelVerbose, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelWarning, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelInfo, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelDebug, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelVerbose, "TagVerbose", "More detailed information to help of debug.", 1}
#else
    {LogLevelDebug, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelWarning, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelInfo, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelDebug, "Tag_Dummy", "dummy message", 0},
    {LogLevelDebug, LogLevelVerbose, "Tag_Dummy", "dummy message", 0},

    {LogLevelVerbose, LogLevelSilent, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelError, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelWarning, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelInfo, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelDebug, "Tag_Dummy", "dummy message", 0},
    {LogLevelVerbose, LogLevelVerbose, "Tag_Dummy", "dummy message", 0}
#endif
};

class EASYHTTPCPPCORELOGXParameterizedTest : public CoreLoggerUnitTest,
public testing::WithParamInterface<EASYHTTPCPPCORELOGXParameterizedTestParam> {
};
INSTANTIATE_TEST_CASE_P(CoreLoggerUnitTest, EASYHTTPCPPCORELOGXParameterizedTest,
        testing::ValuesIn(EASYHTTPCPPCORELOGXParameterizedTestParams));

// EASYHTTPCPP_LOG_*マクロはLogLevelによってログ出力が制御できる
// condition: Macro is switched by the NDEBUG option.
// Debug build: LogLevelVerboseまで出力できる
// Release build: LogLevelInfoまで出力できる

TEST_P(EASYHTTPCPPCORELOGXParameterizedTest, EASYHTTPCPPCORELOGX_PassesExpectedParamsToMockChannel)
{
    // Given: CoreLoggerとMockChannelを準備
    CoreLogger::getInstance()->setLoggingLevel(GetParam().m_setLoggingLevel);

    // Poco::Message に保存する
    Poco::Message pocoMessage;
    MockPocoChannel::Ptr pMockChannel = new MockPocoChannel;
    if (GetParam().m_logCalledTimes == 1) {
        EXPECT_CALL(*pMockChannel, log(testing::_)).Times(GetParam().m_logCalledTimes).
                WillOnce(testing::SaveArg<0>(&pocoMessage));
    } else {
        EXPECT_CALL(*pMockChannel, log(testing::_)).Times(GetParam().m_logCalledTimes);
    }
    Poco::Logger& logger = Poco::Logger::get(CoreLogger::getInstance()->getLogWriter()->getName());
    logger.setChannel(pMockChannel);

    // When: EASYHTTPCPP_LOGマクロを呼び出す
    switch (GetParam().m_messageLogLevel) {
        case LogLevelError:
            EASYHTTPCPP_LOG_E(GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelWarning:
            EASYHTTPCPP_LOG_W(GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelInfo:
            EASYHTTPCPP_LOG_I(GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelDebug:
            EASYHTTPCPP_LOG_D(GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelVerbose:
            EASYHTTPCPP_LOG_V(GetParam().m_tag, GetParam().m_message.c_str());
            break;
        default:
            break;
    }

    // Then: logが呼ばれた時、設定したparameterで作ったMessageとPocoが作成したMessageのフォーマットが同じである
    if (GetParam().m_logCalledTimes == 1) {
        EXPECT_EQ(ConvertToPocoMessagePriority[GetParam().m_messageLogLevel], pocoMessage.getPriority());
        std::string messageRegularExpression = StringUtil::format(ExpectedRegexFormat, GetParam().m_tag.c_str(),
                ConvertToLogLevelChars[GetParam().m_messageLogLevel], GetParam().m_message.c_str());
        Poco::RegularExpression re(messageRegularExpression);
        EXPECT_TRUE(re.match(pocoMessage.getText().c_str()));
    }
}

// EASYHTTPCPP_LOG_*マクロはLogLevelによってログ出力が制御できる
// マクロに format を使用する
// condition: Macro is switched by the NDEBUG option.
// Debug build: LogLevelVerboseまで出力できる
// Release build: LogLevelInfoまで出力できる

TEST_P(EASYHTTPCPPCORELOGXParameterizedTest, EASYHTTPCPPCORELOGXWithFormatter_PassesExpectedParamsToMockChannel)
{
    // Given: CoreLoggerとMockChannelを準備
    CoreLogger::getInstance()->setLoggingLevel(GetParam().m_setLoggingLevel);

    // Poco::Message に保存する
    Poco::Message pocoMessage;
    MockPocoChannel::Ptr pMockChannel = new MockPocoChannel;
    if (GetParam().m_logCalledTimes == 1) {
        EXPECT_CALL(*pMockChannel, log(testing::_)).Times(GetParam().m_logCalledTimes).
                WillOnce(testing::SaveArg<0>(&pocoMessage));
    } else {
        EXPECT_CALL(*pMockChannel, log(testing::_)).Times(GetParam().m_logCalledTimes);
    }
    Poco::Logger& logger = Poco::Logger::get(CoreLogger::getInstance()->getLogWriter()->getName());
    logger.setChannel(pMockChannel);

    // When: EASYHTTPCPP_LOGマクロを呼び出す
    switch (GetParam().m_messageLogLevel) {
        case LogLevelError:
            EASYHTTPCPP_LOG_E(GetParam().m_tag, "%s", GetParam().m_message.c_str());
            break;
        case LogLevelWarning:
            EASYHTTPCPP_LOG_W(GetParam().m_tag, "%s", GetParam().m_message.c_str());
            break;
        case LogLevelInfo:
            EASYHTTPCPP_LOG_I(GetParam().m_tag, "%s", GetParam().m_message.c_str());
            break;
        case LogLevelDebug:
            EASYHTTPCPP_LOG_D(GetParam().m_tag, "%s", GetParam().m_message.c_str());
            break;
        case LogLevelVerbose:
            EASYHTTPCPP_LOG_V(GetParam().m_tag, "%s", GetParam().m_message.c_str());
            break;
        default:
            break;
    }

    // Then: logが呼ばれた時、設定したparameterで作ったMessageとPocoが作成したMessageのフォーマットが同じである
    if (GetParam().m_logCalledTimes == 1) {
        EXPECT_EQ(ConvertToPocoMessagePriority[GetParam().m_messageLogLevel], pocoMessage.getPriority());
        std::string messageRegularExpression = StringUtil::format(ExpectedRegexFormat, GetParam().m_tag.c_str(),
                ConvertToLogLevelChars[GetParam().m_messageLogLevel], GetParam().m_message.c_str());
        Poco::RegularExpression re(messageRegularExpression);
        EXPECT_TRUE(re.match(pocoMessage.getText().c_str()));
    }
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
