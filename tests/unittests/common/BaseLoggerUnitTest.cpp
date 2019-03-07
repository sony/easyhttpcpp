/*
 * Copyright 2017 Sony Corporation
 */

#include <typeinfo>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"
#include "Poco/Logger.h"
#include "Poco/Message.h"
#include "Poco/RegularExpression.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/BaseLogger.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/DefaultLogWriter.h"
#include "easyhttpcpp/common/LogLevel.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "StdCLogCapture.h"

namespace easyhttpcpp {
namespace common {
namespace test {

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::StdCLogCapture;

#ifndef NDEBUG
static const LogLevel ExpectedDefaultLogLevel = LogLevelVerbose;
#else
static const LogLevel ExpectedDefaultLogLevel = LogLevelWarning;
#endif

static const char LogLevelChar[] = "VDIWE";
static const std::string ExpectedRegularExpressionFormat = ".* EASYHTTPCPP_%s LogLv=%c line=[0-9]+ %s.*";

class BaseLoggerUnitTest : public testing::Test {
protected:

    Poco::SharedPtr<BaseLogger> m_pBaseLogger;

    void SetUp()
    {
        ASSERT_NO_THROW(m_pBaseLogger = new BaseLogger()) << "Failed to create BaseLogger.";
    }

    void TearDown()
    {
        // Poco::Logger cleanup
        // Poco::Logger::shutdown()は、LoggerMapをclearする為、subclassのLoggerでresetToDefaultする必要がある
        Poco::Logger::shutdown();
        CoreLogger::getInstance()->resetToDefaults();
        m_pBaseLogger->resetToDefaults();
    }

    class MockLogWriter : public LogWriter {
    public:
        typedef Poco::AutoPtr<MockLogWriter> Ptr;

        MOCK_CONST_METHOD0(getName, const std::string&());
        MOCK_METHOD4(log, void(const std::string& tag, easyhttpcpp::common::LogLevel level, unsigned int line,
                const std::string& message));
    };

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

static const LogLevel LogLevelParams[] = {
    LogLevelSilent,
    LogLevelError,
    LogLevelWarning,
    LogLevelInfo,
    LogLevelDebug,
    LogLevelVerbose
};

class GetLoggingLevelParameterizedTest : public BaseLoggerUnitTest, public ::testing::WithParamInterface<LogLevel> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, GetLoggingLevelParameterizedTest, ::testing::ValuesIn(LogLevelParams));

// getLoggingLevelは、setLoggingLevelで設定したLogLevelを取得できる

TEST_P(GetLoggingLevelParameterizedTest, setLoggingLevel_CanUpdateLogLevel_WhenSetValidLogLevel)
{
    // Given: -
    // When: setLoggingLevel()を呼ぶ
    m_pBaseLogger->setLoggingLevel(GetParam());

    // Then: setしたLogLevelとgetしたLogLevelが同じ
    EXPECT_EQ(GetParam(), m_pBaseLogger->getLoggingLevel());
}

// BaseLoggerにLogLevelを設定していない時、getLoggingLevelはdefaultのLogLevelを返す
// Condition: ExpectedDefaultLogLevel is switched by the NDEBUG option.
// Debug build: LogLevelVerbose
// Release build: LogLevelWarning

TEST_F(BaseLoggerUnitTest, getLoggingLevel_ReturnsDefaultLogLevel_WhenNoSetLoggingLevel)
{
    // Given: -

    // When: getLoggingLevelを呼ぶ
    // Then: defaultのLogLevelが取得できる
    EXPECT_EQ(ExpectedDefaultLogLevel, m_pBaseLogger->getLoggingLevel());
}

// setLogWriterはLogWriterを設定できる

TEST_F(BaseLoggerUnitTest, setLogWriter_Succeeds)
{
    // Given: create MockLogWriter
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    EXPECT_CALL(*pMockWriter, log(std::string("tag"), LogLevelError, 0, std::string("dummy"))).
            WillOnce(::testing::Return());

    // When: setLogWriterでmockLogWriterを設定する
    m_pBaseLogger->setLogWriter(pMockWriter);

    // Then: 設定したLogWriterのlog()が呼ばれる
    m_pBaseLogger->log("tag", LogLevelError, 0, "dummy");
}

// setLogWriterはNULLを与えた時LogWriterを切り替えない

TEST_F(BaseLoggerUnitTest, setLogWriter_NotChangeLogWriter_WhenSetNull)
{
    // Given: create MockLogWriter
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    EXPECT_CALL(*pMockWriter, log(std::string("tag"), LogLevelError, 0, std::string("dummy"))).
            WillOnce(::testing::Return());
    m_pBaseLogger->setLogWriter(pMockWriter);

    // When: setLogWriterでNULLを設定する
    m_pBaseLogger->setLogWriter(NULL);

    // Then: 有効なLogWriterのlog()が呼ばれる
    m_pBaseLogger->log("tag", LogLevelError, 0, "dummy");
}

// getLogWriterはBaseLoggerに設定されているDefaultLogWriterを取得できる

TEST_F(BaseLoggerUnitTest, getLogWriter_ReturnsDefaultLogWriter_WhenNotSetLogWriter)
{
    // Given: DefaultLogWriterのtype_infoを準備
    const std::type_info& expectedId = typeid (DefaultLogWriter);

    // When: getLogWriterでLogWriterを取得する
    LogWriter* pWriter = m_pBaseLogger->getLogWriter().get();
    const std::type_info& actualId = typeid (*pWriter);

    // Then: DefaultLogWriterが取得できる
    EXPECT_EQ(expectedId, actualId);
    EXPECT_STREQ(expectedId.name(), actualId.name());
}

// getLogWriterはsetLogWriterで設定したLogWriterを取得できる

TEST_F(BaseLoggerUnitTest, getLogWriter_ReturnsTheSetLogWriter_WhenAfterSetLogWriter)
{
    // Given: MockLogWriterを設定
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    m_pBaseLogger->setLogWriter(pMockWriter);
    const std::type_info& expectedId = typeid (MockLogWriter);

    // When: getLogWriterでLogWriterを取得する
    LogWriter* pWriter = m_pBaseLogger->getLogWriter().get();
    const std::type_info& actualId = typeid (*pWriter);

    // Then: MockLogWriterが取得できる
    EXPECT_EQ(expectedId, actualId);
    EXPECT_STREQ(expectedId.name(), actualId.name());
}

class LogWithSetLoggingLevelTestParam {
public:
    LogLevel m_setLogLevel;
    LogLevel m_logLevelOfMessage;
    unsigned int m_logCalledTimes;
};

static const LogWithSetLoggingLevelTestParam LogWithLogLevelErrorTestParams[] = {
    // m_setLogLevel, m_logLevelOfMessage, m_logCalledTimes
    {LogLevelSilent, LogLevelSilent, 1/*No use case.*/},
    {LogLevelSilent, LogLevelError, 0},
    {LogLevelSilent, LogLevelWarning, 0},
    {LogLevelSilent, LogLevelInfo, 0},
    {LogLevelSilent, LogLevelDebug, 0},
    {LogLevelSilent, LogLevelVerbose, 0},
    {LogLevelError, LogLevelSilent, 1/*No use case.*/},
    {LogLevelError, LogLevelError, 1},
    {LogLevelError, LogLevelWarning, 0},
    {LogLevelError, LogLevelInfo, 0},
    {LogLevelError, LogLevelDebug, 0},
    {LogLevelError, LogLevelVerbose, 0},
    {LogLevelWarning, LogLevelSilent, 1/*No use case.*/},
    {LogLevelWarning, LogLevelError, 1},
    {LogLevelWarning, LogLevelWarning, 1},
    {LogLevelWarning, LogLevelInfo, 0},
    {LogLevelWarning, LogLevelDebug, 0},
    {LogLevelWarning, LogLevelVerbose, 0},
    {LogLevelInfo, LogLevelSilent, 1/*No use case.*/},
    {LogLevelInfo, LogLevelError, 1},
    {LogLevelInfo, LogLevelWarning, 1},
    {LogLevelInfo, LogLevelInfo, 1},
    {LogLevelInfo, LogLevelDebug, 0},
    {LogLevelInfo, LogLevelVerbose, 0},
    {LogLevelDebug, LogLevelSilent, 1/*No use case.*/},
    {LogLevelDebug, LogLevelError, 1},
    {LogLevelDebug, LogLevelWarning, 1},
    {LogLevelDebug, LogLevelInfo, 1},
    {LogLevelDebug, LogLevelDebug, 1},
    {LogLevelDebug, LogLevelVerbose, 0},
    {LogLevelVerbose, LogLevelSilent, 1/*No use case.*/},
    {LogLevelVerbose, LogLevelError, 1},
    {LogLevelVerbose, LogLevelWarning, 1},
    {LogLevelVerbose, LogLevelInfo, 1},
    {LogLevelVerbose, LogLevelDebug, 1},
    {LogLevelVerbose, LogLevelVerbose, 1},
};

class LogWithSetLoggingLevelParameterizedTest : public BaseLoggerUnitTest,
public ::testing::WithParamInterface<LogWithSetLoggingLevelTestParam> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, LogWithSetLoggingLevelParameterizedTest,
        ::testing::ValuesIn(LogWithLogLevelErrorTestParams));

// logはLogLevelにより出力を制御できる

TEST_P(LogWithSetLoggingLevelParameterizedTest, log_CanControl_OutputByLogLevel)
{
    // Given: LogLevelを設定し、MockLogWriterを設定
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    m_pBaseLogger->setLoggingLevel(GetParam().m_setLogLevel);
    EXPECT_CALL(*pMockWriter, log(::testing::_, GetParam().m_logLevelOfMessage, ::testing::_, ::testing::_)).
            Times(GetParam().m_logCalledTimes);
    m_pBaseLogger->setLogWriter(pMockWriter);

    // When: logを呼ぶ
    m_pBaseLogger->log("Tag_Dummy", GetParam().m_logLevelOfMessage, 0, "dummy message");

    // Then: LogLevelによりLogWriter::logの呼び出しを制御できる
}

class LogArgumentParameterizedTestParam {
public:
    std::string m_tag;
    LogLevel m_level;
    unsigned int m_line;
    std::string m_message;
    unsigned int m_logCalledTimes;
};

static const LogArgumentParameterizedTestParam LogArgumentParameterizeTestParams[] = {
    // m_tag, m_level, m_line, m_message, m_logCalledTimes
    {"Tag_Silent", LogLevelSilent, 1, "dummy message", 1/*No use case.*/},
    {"Tag_Silent", LogLevelSilent, 2, "", 1/*No use case.*/},
    {"Tag_Silent", LogLevelSilent, 3, " ", 1/*No use case.*/},
    {"", LogLevelSilent, 4, "", 1/*No use case.*/},
    {"", LogLevelSilent, 5, " ", 1/*No use case.*/},
    {" ", LogLevelSilent, 6, " ", 1/*No use case.*/},

    {"Tag_Error", LogLevelError, 1, "Something terrible happened.", 1},
    {"Tag_Error", LogLevelError, 2, "", 1},
    {"Tag_Error", LogLevelError, 3, " ", 1},
    {"", LogLevelError, 4, "", 1},
    {"", LogLevelError, 5, " ", 1},
    {" ", LogLevelError, 6, " ", 1},

    {"Tag_Warning", LogLevelWarning, 1, "Internal warning occurred.", 1},
    {"Tag_Warning", LogLevelWarning, 2, "", 1},
    {"Tag_Warning", LogLevelWarning, 3, " ", 1},
    {"", LogLevelWarning, 4, "", 1},
    {"", LogLevelWarning, 5, " ", 1},
    {" ", LogLevelWarning, 6, " ", 1},

    {"Tag_Information", LogLevelInfo, 3, "Information of processing.", 1},
    {"Tag_Information", LogLevelInfo, 2, "", 1},
    {"Tag_Information", LogLevelInfo, 3, " ", 1},
    {"", LogLevelInfo, 4, "", 1},
    {"", LogLevelInfo, 5, " ", 1},
    {" ", LogLevelInfo, 6, " ", 1},

    {"Tag_Debug", LogLevelDebug, 4, "Information to help of debug.", 1},
    {"Tag_Debug", LogLevelDebug, 2, "", 1},
    {"Tag_Debug", LogLevelDebug, 3, " ", 1},
    {"", LogLevelDebug, 4, "", 1},
    {"", LogLevelDebug, 5, " ", 1},
    {" ", LogLevelDebug, 6, " ", 1},

    {"Tag_Verbose", LogLevelVerbose, 5, "More detailed information to help of debug.", 1},
    {"Tag_Verbose", LogLevelVerbose, 2, "", 1},
    {"Tag_Verbose", LogLevelVerbose, 3, " ", 1},
    {"", LogLevelVerbose, 4, "", 1},
    {"", LogLevelVerbose, 5, " ", 1},
    {" ", LogLevelVerbose, 6, " ", 1},

    {"Tag_Verbose", LogLevelVerbose, UINT_MAX, "More detailed information to help of debug.", 1}
};

class LogArgumentParameterizedTest : public BaseLoggerUnitTest,
public ::testing::WithParamInterface<LogArgumentParameterizedTestParam> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, LogArgumentParameterizedTest,
        ::testing::ValuesIn(LogArgumentParameterizeTestParams));

// BaseLogger::logに渡したパラメータはそのままLogWriterに渡す

TEST_P(LogArgumentParameterizedTest, log_PassParameters_SetAsItIsInLogWriter)
{
    // Given: LogLevelを設定し、MockLogWriterを設定
    m_pBaseLogger->setLoggingLevel(LogLevelVerbose);
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    EXPECT_CALL(*pMockWriter, log(GetParam().m_tag, GetParam().m_level, GetParam().m_line, GetParam().m_message))
            .Times(GetParam().m_logCalledTimes);
    m_pBaseLogger->setLogWriter(pMockWriter);

    // When: logを呼ぶ
    m_pBaseLogger->log(GetParam().m_tag, GetParam().m_level, GetParam().m_line, GetParam().m_message);

    // Then: logはセットしたパラメータをそのままLogWriterに渡す
}

static const LogArgumentParameterizedTestParam LogWithDefaultLogWriterTestParams[] = {
    {"Tag_Silent", LogLevelSilent, 0, "dummy message", 0/*No use case.*/},
    {"Tag_Error", LogLevelError, 1, "Something terrible happened.", 1},
    {"Tag_Warning", LogLevelWarning, 2, "Internal warning occurred.", 1},
#ifndef NDEBUG
    {"Tag_Information", LogLevelInfo, 3, "Information of processing.", 1},
    {"Tag_Debug", LogLevelDebug, 4, "Information to help of debug.", 1},
    {"Tag_Verbose", LogLevelVerbose, 5, "More detailed information to help of debug.", 1}
#else
    {"Tag_Information", LogLevelInfo, 3, "Information of processing.", 0},
    {"Tag_Debug", LogLevelDebug, 4, "Information to help of debug.", 0},
    {"Tag_Verbose", LogLevelVerbose, 5, "More detailed information to help of debug.", 0}
#endif
};

class LogWithDefaultLogWriterTest : public BaseLoggerUnitTest,
public ::testing::WithParamInterface<LogArgumentParameterizedTestParam> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, LogWithDefaultLogWriterTest,
        ::testing::ValuesIn(LogWithDefaultLogWriterTestParams));

// DefaultLogWriter, DefaultLogLevelの時、logはPoco::Loggerを呼び出す
// Condition: DefaultLogLevel is switched by the NDEBUG option.
// Debug build: LogLevelVerbose
// Release build: LogLevelWarning

TEST_P(LogWithDefaultLogWriterTest, log_UseDefaultLogWriter_WhenNotSetLogWriter)
{
    // Given: MockPocoChannelを設定
    MockPocoChannel::Ptr pMockChannel = new MockPocoChannel;
    EXPECT_CALL(*pMockChannel, log(::testing::_)).Times(GetParam().m_logCalledTimes);
    Poco::Logger& logger = Poco::Logger::get(m_pBaseLogger->getLogWriter()->getName());
    logger.setChannel(pMockChannel);

    // When: logを呼ぶ
    m_pBaseLogger->log(GetParam().m_tag, GetParam().m_level, GetParam().m_line, GetParam().m_message);

    // Then: Debug の時はLogLevelVerboseまで、Release の時はWarningまでがPoco::Loggerを呼ぶ
}

// resetToDefaultsは、LogLevelとLogWriterをdefaultにする

TEST_F(BaseLoggerUnitTest, resetToDefaults_Succeeds)
{
    // Given: BaseLoggerにdefaultではない設定をする
    m_pBaseLogger->setLoggingLevel(LogLevelError);
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    m_pBaseLogger->setLogWriter(pMockWriter);

    // When: resetToDefaults()を呼ぶ
    m_pBaseLogger->resetToDefaults();

    // Then: BaseLoggerが初期化される
    // ExpectedDefaultLogLevel is switched by the NDEBUG option.
    EXPECT_EQ(ExpectedDefaultLogLevel, m_pBaseLogger->getLoggingLevel());

    const std::type_info& expectedId = typeid (DefaultLogWriter);
    LogWriter* pWriter = m_pBaseLogger->getLogWriter().get();
    const std::type_info& actualId = typeid (*pWriter);
    EXPECT_EQ(expectedId, actualId);
    EXPECT_STREQ(expectedId.name(), actualId.name());
}

class EASYHTTPCPPLOGXParameterizedTestParam {
public:
    std::string m_tag;
    LogLevel m_level;
    std::string m_message;

    unsigned int m_logCalledTimes;
};

static const EASYHTTPCPPLOGXParameterizedTestParam EASYHTTPCPPLOGXParameterizedTestParams[] = {
    {"Tag_Error", LogLevelError, "Something terrible happened.", 1},
    {"Tag_Warning", LogLevelWarning, "Internal warning occurred.", 1},
    {"Tag_Information", LogLevelInfo, "Information of processing.", 1},
#ifndef NDEBUG
    {"Tag_Debug", LogLevelDebug, "Information to help of debug.", 1},
    {"Tag_Verbose", LogLevelVerbose, "More detailed information to help of debug.", 1}
#else
    {"Tag_Debug", LogLevelDebug, "Information to help of debug.", 0},
    {"Tag_Verbose", LogLevelVerbose, "More detailed information to help of debug.", 0}
#endif
};

class EASYHTTPCPPLOGXParameterizedTest : public BaseLoggerUnitTest,
public ::testing::WithParamInterface<EASYHTTPCPPLOGXParameterizedTestParam> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, EASYHTTPCPPLOGXParameterizedTest,
        ::testing::ValuesIn(EASYHTTPCPPLOGXParameterizedTestParams));

// EASYHTTPCPP_BASE_LOG_*マクロはLogWriter#logにパラメータをそのまま渡すことができる
// condition: Macro is switched by the NDEBUG option.
// Debug build: LogLevelVerboseまで出力できる
// Release build: LogLevelInfoまで出力できる

TEST_P(EASYHTTPCPPLOGXParameterizedTest, EASYHTTPCPPLOGX_PassesExpectedParamsToLogWriter)
{
    // Given: LoggerとLogWriterを準備
    BaseLogger logger;
    MockLogWriter::Ptr pMockWriter = new MockLogWriter;
    EXPECT_CALL(*pMockWriter, log(GetParam().m_tag, GetParam().m_level, ::testing::Gt(0), GetParam().m_message))
            .Times(GetParam().m_logCalledTimes);
    logger.setLoggingLevel(LogLevelVerbose);
    logger.setLogWriter(pMockWriter);

    // When: EASYHTTPCPP_LOGマクロを呼び出す
    switch (GetParam().m_level) {
        case LogLevelError:
            EASYHTTPCPP_BASE_LOG_E(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelWarning:
            EASYHTTPCPP_BASE_LOG_W(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelInfo:
            EASYHTTPCPP_BASE_LOG_I(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelDebug:
            EASYHTTPCPP_BASE_LOG_D(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelVerbose:
            EASYHTTPCPP_BASE_LOG_V(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        default:
            ADD_FAILURE() << "Failed to call EASYHTTPCPP_BASE_LOG_X macro's. It's unknown LogLevel.";
            break;
    }

    // Then: As described below:
    // Debug:MockLogWriter::logが呼ばれる
    // Release: EASYHTTPCPP_BASE_LOG_D/EASYHTTPCPP_BASE_LOG_V 以外は、MockLogWriter::logが呼ばれる
}

class EASYHTTPCPPLOGXRedirectParameterizedTestParam {
public:
    std::string m_tag;
    LogLevel m_level;
    std::string m_message;

    unsigned int m_logCalledTimes;
};

static const EASYHTTPCPPLOGXRedirectParameterizedTestParam EASYHTTPCPPLOGXRedirectParameterizedTestParams[] = {
    {"Tag_Error", LogLevelError, "Something terrible happened.", 1},
    {"Tag_Warning", LogLevelWarning, "Internal warning occurred.", 1},
    {"Tag_Information", LogLevelInfo, "Information of processing.", 1},
#ifndef NDEBUG
    {"Tag_Debug", LogLevelDebug, "Information to help of debug.", 1},
    {"Tag_Verbose", LogLevelVerbose, "Output verbose log.", 1}
#else
    {"Tag_Debug", LogLevelDebug, "Information to help of debug.", 0},
    {"Tag_Verbose", LogLevelVerbose, "More detailed information to help of debug.", 0}
#endif
};

class EASYHTTPCPPLOGXRedirectParameterizedTest : public BaseLoggerUnitTest,
public ::testing::WithParamInterface<EASYHTTPCPPLOGXRedirectParameterizedTestParam> {
};
INSTANTIATE_TEST_CASE_P(BaseLoggerUnitTest, EASYHTTPCPPLOGXRedirectParameterizedTest,
        ::testing::ValuesIn(EASYHTTPCPPLOGXRedirectParameterizedTestParams));

// EASYHTTPCPP_BASE_LOG_*マクロはログ出力できる
// condition: Macro is switched by the NDEBUG option.
// Debug build: LogLevelVerboseまで出力できる
// Release build: LogLevelInfoまで出力できる

TEST_P(EASYHTTPCPPLOGXRedirectParameterizedTest, EASYHTTPCPPLOGX_CanBeLogOutput_ActuallyUsingThePocoLogger)
{
    // Given: リダイレクトの準備
    BaseLogger logger;
    // log levelはLogLevelVerbose
    logger.setLoggingLevel(LogLevelVerbose);

    StdCLogCapture logCapture(1024 * 10);
    logCapture.startCapture();

    // When: EASYHTTPCPP_LOGマクロを呼び出す
    switch (GetParam().m_level) {
        case LogLevelError:
            EASYHTTPCPP_BASE_LOG_E(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelWarning:
            EASYHTTPCPP_BASE_LOG_W(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelInfo:
            EASYHTTPCPP_BASE_LOG_I(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelDebug:
            EASYHTTPCPP_BASE_LOG_D(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        case LogLevelVerbose:
            EASYHTTPCPP_BASE_LOG_V(&logger, GetParam().m_tag, GetParam().m_message.c_str());
            break;
        default:
            ADD_FAILURE() << "Failed to call EASYHTTPCPP_BASE_LOG_X macro's. It's unknown LogLevel.";
            break;
    }
    // End capture.
    logCapture.endCapture();

    // Then: リダイレクトされたlogが正しく出力されていること
    // ファイルから全行取得
    StdCLogCapture::CapturedLinesVec redirectLines = logCapture.getCapture();
    ASSERT_EQ(GetParam().m_logCalledTimes, redirectLines.size());

    if (GetParam().m_logCalledTimes == 1) {
        // 比較
        StdCLogCapture::CapturedLinesVec::iterator it = redirectLines.begin();

        // 正規表現の文字列を作成し、文字列を比較する
        std::string messageRegex = StringUtil::format(ExpectedRegularExpressionFormat.c_str(), GetParam().m_tag.c_str(),
                LogLevelChar[GetParam().m_level], GetParam().m_message.c_str());
        Poco::RegularExpression re(messageRegex);
        EXPECT_TRUE(re.match(*it));
    }
}

// Messageが10MB以上のログも出力できる

TEST_F(BaseLoggerUnitTest, messageSize_BoundaryCheck)
{
    // Given: create log message
    BaseLogger logger;
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
    EASYHTTPCPP_BASE_LOG_E(&logger, tag, message.c_str());

    // Then: 落ちないこと
    capture.endCapture();
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
