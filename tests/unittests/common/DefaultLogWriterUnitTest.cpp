/*
 * Copyright 2017 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/AutoPtr.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/RegularExpression.h"
#ifdef _WIN32
#include "Poco/WindowsConsoleChannel.h"
#else
#include "Poco/ConsoleChannel.h"
#endif // _WIN32

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/DefaultLogWriter.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "StdCLogCapture.h"

using easyhttpcpp::testutil::StdCLogCapture;

namespace easyhttpcpp {
namespace common {
namespace test {

#define EXPECT_MESSAGE_FORMAT(tag, level, line, msg) \
        StringUtil::format("EASYHTTPCPP_%s LogLv=%c line=%u %s", tag, level, line, msg)

class DefaultLogWriterUnitTest : public testing::Test {
protected:
    static const std::string PatternOfLogWriterName;

    virtual void TearDown()
    {
        // Poco::Logger cleanup
        // Poco::Logger::shutdown()は、LoggerMapをclearする為、subclassのLoggerでresetToDefaultする必要がある
        Poco::Logger::shutdown();
        CoreLogger::getInstance()->resetToDefaults();
    }
};
const std::string DefaultLogWriterUnitTest::PatternOfLogWriterName = "EASYHTTPCPPDefaultLogWriter_[0-9]+_[0-9]+";

class MockPocoChannel : public Poco::Channel {
public:
    MOCK_METHOD1(log, void(const Poco::Message& msg));
};

TEST_F(DefaultLogWriterUnitTest, constructor_SetsValidSettings)
{
    // Given: -
    // When: Create DefaultLogWriter.
    DefaultLogWriter::Ptr pWriter = new DefaultLogWriter();

    // Then: Check property of Poco::Logger.
    Poco::Logger& logger = Poco::Logger::get(pWriter->getName());
    Poco::FormattingChannel* pFCchannel = static_cast<Poco::FormattingChannel*> (logger.getChannel());
#ifndef _WIN32
    Poco::ColorConsoleChannel* pChannel = static_cast<Poco::ColorConsoleChannel*> (pFCchannel->getChannel());
#else
    Poco::WindowsColorConsoleChannel* pChannel =
            static_cast<Poco::WindowsColorConsoleChannel*> (pFCchannel->getChannel());
#endif // !_WIN32
    Poco::Formatter* pFormatter = static_cast<Poco::PatternFormatter*> (pFCchannel->getFormatter());

    std::string enableColors = pChannel->getProperty("enableColors");
    std::string errorColor = pChannel->getProperty("errorColor");
    std::string warningColor = pChannel->getProperty("warningColor");
    std::string infoColor = pChannel->getProperty("informationColor");
    std::string debugColor = pChannel->getProperty("debugColor");
    std::string traceColor = pChannel->getProperty("traceColor");
    std::string format = pFormatter->getProperty(Poco::PatternFormatter::PROP_PATTERN);

    EXPECT_STREQ("true", enableColors.c_str());
    EXPECT_STREQ("lightRed", errorColor.c_str());
    EXPECT_STREQ("brown", warningColor.c_str());

    // not verify results of getProperty for "informationColor", "debugColor", "traceColor" on Windows
    // because the results are depending on Windows console font color settings, it is not "default".
#ifndef _WIN32
    EXPECT_STREQ("default", infoColor.c_str());
    EXPECT_STREQ("default", debugColor.c_str());
    EXPECT_STREQ("default", traceColor.c_str());
#endif // !_WIN32
    EXPECT_STREQ("%L%Y/%m/%d %H:%M:%S [%P:%I] %t", format.c_str());

    Poco::RegularExpression re(PatternOfLogWriterName);
    EXPECT_TRUE(re.match(pWriter->getName()));
}

class LogTestParam {
public:
    // log parameter
    std::string m_tag;
    LogLevel m_logLevel;
    unsigned int m_line;
    std::string m_message;

    // expected value
    char m_expectLogLevelChar;
    Poco::Message::Priority m_expectedPocoPriority;
};

static LogTestParam LogTestParams[] = {
    /* m_tag, m_logLevel, m_line, m_message, m_expectLogLevelChar, m_expectedPriority*/
    {"", LogLevelError, 1, "dummy Message", 'E', Poco::Message::PRIO_ERROR},
    {" ", LogLevelError, 2, "dummy Message", 'E', Poco::Message::PRIO_ERROR},
    {"tag", LogLevelError, 3, "", 'E', Poco::Message::PRIO_ERROR},
    {"tag", LogLevelError, 4, " ", 'E', Poco::Message::PRIO_ERROR},
    {"tag", LogLevelError, 5, "dummy Message", 'E', Poco::Message::PRIO_ERROR},
    {"tag", LogLevelWarning, 6, "dummy Message", 'W', Poco::Message::PRIO_WARNING},
    {"tag", LogLevelInfo, 7, "dummy Message", 'I', Poco::Message::PRIO_INFORMATION},
    {"tag", LogLevelDebug, 8, "dummy Message", 'D', Poco::Message::PRIO_DEBUG},
    {"tag", LogLevelVerbose, 9, "dummy Message", 'V', Poco::Message::PRIO_TRACE}
};

class LogParameterizedTest : public DefaultLogWriterUnitTest, public ::testing::WithParamInterface<LogTestParam> {
};
INSTANTIATE_TEST_CASE_P(DefaultLogWriterUnitTest, LogParameterizedTest, ::testing::ValuesIn(LogTestParams));

TEST_P(LogParameterizedTest, log_CanBePassedParameterInPocoLogger_WhenLogLevelSpecified)
{
    // Given: Set MockChannel to Poco::Logger of DefaultLogWriter.
    DefaultLogWriter::Ptr pWriter = new DefaultLogWriter();

    Poco::Logger& logger = Poco::Logger::get(pWriter->getName());
    Poco::AutoPtr<MockPocoChannel> pMockPocoChannel = new MockPocoChannel;
    Poco::Message pocoMessage;
    EXPECT_CALL(*pMockPocoChannel, log(testing::_)).WillOnce(testing::SaveArg<0>(&pocoMessage));
    logger.setChannel(pMockPocoChannel);

    // When: Calls DefaultLogWriter#log()
    pWriter->log(GetParam().m_tag, GetParam().m_logLevel, GetParam().m_line, GetParam().m_message);

    // Then: log method of Poco is to make sure it was called.
    std::string expectedPocoMessageText = EXPECT_MESSAGE_FORMAT(GetParam().m_tag.c_str(),
            GetParam().m_expectLogLevelChar, GetParam().m_line, GetParam().m_message.c_str());
    EXPECT_STREQ(pWriter->getName().c_str(), pocoMessage.getSource().c_str());
    EXPECT_STREQ(expectedPocoMessageText.c_str(), pocoMessage.getText().c_str());
    EXPECT_EQ(GetParam().m_expectedPocoPriority, pocoMessage.getPriority());
}

TEST_F(DefaultLogWriterUnitTest, log_PocoLoggerIsNeverCalled_WhenLogLevelSilent)
{
    // Given: Set MockChannel to Poco::Logger of DefaultLogWriter.
    DefaultLogWriter::Ptr pWriter = new DefaultLogWriter();

    Poco::Logger& logger = Poco::Logger::get(pWriter->getName());
    Poco::AutoPtr<MockPocoChannel> pMockChannel = new MockPocoChannel;
    EXPECT_CALL(*pMockChannel, log(testing::_)).Times(0);
    logger.setChannel(pMockChannel);

    // When: Calls DefaultLogWriter#log()
    pWriter->log("tag_dummy", LogLevelSilent, 1, "dummy Message");

    // Then: Never called Poco::Channell::log()
}

TEST_F(DefaultLogWriterUnitTest, messageSize_BoundaryCheck)
{
    // Given: create log message
    DefaultLogWriter::Ptr pWriter = new DefaultLogWriter();
    std::string tag = "";
    std::string message = "";

    // 10 * 1024 * 8 = 81,920
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 1024; j++) {
            tag += "1234567890";
        }
    }

    // 10 * 1024 * 1024 = 10,485,760
    for (size_t i = 0; i < 1024; i++) {
        for (size_t j = 0; j < 1024; j++) {
            message += "1234567890";
        }
    }
    // std::clog の出力先をコンソールから MemoryStream に Redirect して捨てる
    StdCLogCapture capture(1); // 出力先の stream buffer:1byte
    capture.startCapture();

    // When: マクロ実行
    pWriter->log(tag, LogLevelError, UINT_MAX, message);

    // Then: 落ちないこと
    capture.endCapture();
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
