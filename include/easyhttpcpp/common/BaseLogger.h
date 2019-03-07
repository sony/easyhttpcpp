/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_BASELOGGER_H_INCLUDED
#define EASYHTTPCPP_COMMON_BASELOGGER_H_INCLUDED

#include "easyhttpcpp/common/CommonExports.h"
#include "easyhttpcpp/common/LogLevel.h"
#include "easyhttpcpp/common/LogWriter.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {

#ifndef NDEBUG
#define EASYHTTPCPP_BASE_LOG_E(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelError, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_W(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelWarning, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_I(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelInfo, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_D(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelDebug, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_V(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelVerbose, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#else
#define EASYHTTPCPP_BASE_LOG_E(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelError, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_W(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelWarning, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_I(pLogger, pTag, pFormat, ...) \
        easyhttpcpp::common::BaseLogger::safeLog(pLogger, pTag, easyhttpcpp::common::LogLevelInfo, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_D(pLogger, pTag, pFormat, ...)
#define EASYHTTPCPP_BASE_LOG_V(pLogger, pTag, pFormat, ...)
#endif

/**
 * @class BaseLogger BaseLogger.h "easyhttpcpp/common/BaseLogger.h"
 *
 * Base logger with format style support.
 */
class EASYHTTPCPP_COMMON_API BaseLogger {
public:
    BaseLogger();
    virtual ~BaseLogger();

    /**
     * @brief Sets the minimum logging level that overrides the default level.
     *
     * This api can be used to control the amount of logs output by library.
     * The log with a logging level higher than or equal to the logging level specified by this api will be output.
     * In case of release builds, maximum logging level that can be set with this api is ::LogLevelInfo. Higher levels,
     * even if set will be ignored due to possible sensitive nature of the logs.
     *
     * @note To silent all logs, use ::LogLevelSilent.
     * @param level the minimum logging level to use.
     */
    void setLoggingLevel(LogLevel level);

    /**
     * @brief Gets the minimum logging level as set by setLoggingLevel(LogLevel) or default logging level if not set.
     *
     * Default logging levels are ::LogLevelVerbose and ::LogLevelWarning for debug and release builds respectively.
     *
     * @return the logging level.
     */
    LogLevel getLoggingLevel();

    /**
     * @brief Sets @c pWriter as the library log writer overriding the default.
     *
     * The default log writer uses Poco::Logger which by default outputs to @c std::log. This api can be useful if
     * applications want to redirect library log output to say, syslog or file etc.
     *
     * @param pWriter the log writer to use.
     * @note Since library uses the default log writer internally for logging as well, it is recommended to call this
     * api at the very start (before calling any library api).
     */
    void setLogWriter(LogWriter::Ptr pWriter);

    /**
     * @brief Gets the current log writer as set by setLogWriter(LogWriter::Ptr) or default log writer if not set.
     *
     * The default log writer uses Poco::Logger which by default outputs to @c std::log.
     *
     * @return the log writer.
     */
    LogWriter::Ptr getLogWriter() const;

    /**
     * @brief Outputs the log using the current log writer as returned by getLogWriter().
     *
     * @param tag Used to identify the source of a log message. It usually identifies the class where the log call
     * occurs.
     * @param level The priority/level of this log message.
     * @param line Line number of source code to output with log.
     * @param message The message you would like logged.
     */
    void log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message);

    /**
     * @brief If logger is enabled, Outputs the log.
     *
     * @param pLogger Pointer of logger instance.
     * @param tag Used to identify the source of a log message. It usually identifies the class where the log call
     * occurs.
     * @param level The priority/level of this log message.
     * @param line Line number of source code to output with log.
     * @param message The message you would like logged.
     */
    static void safeLog(BaseLogger* pLogger, const std::string& tag, LogLevel level, unsigned int line,
            const std::string& message);

    // internal method; used for testing
    void resetToDefaults();

private:
    static const LogLevel DefaultLogLevel;

    bool isLoggable(LogLevel level);

    LogLevel m_level;
    LogWriter::Ptr m_pWriter;

};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_BASELOGGER_H_INCLUDED */
