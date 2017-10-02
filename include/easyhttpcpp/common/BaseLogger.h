/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_BASELOGGER_H_INCLUDED
#define EASYHTTPCPP_COMMON_BASELOGGER_H_INCLUDED

#include "easyhttpcpp/common/LogLevel.h"
#include "easyhttpcpp/common/LogWriter.h"

namespace easyhttpcpp {
namespace common {

#ifndef NDEBUG
#define EASYHTTPCPP_BASE_LOG_E(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelError, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_W(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelWarning, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_I(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelInfo, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_D(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelDebug, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_V(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelVerbose, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#else
#define EASYHTTPCPP_BASE_LOG_E(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelError, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_W(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelWarning, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_I(logger, pTag, pFormat, ...) logger.log(pTag, easyhttpcpp::common::LogLevelInfo, __LINE__, \
        easyhttpcpp::common::StringUtil::format(pFormat, ##__VA_ARGS__))
#define EASYHTTPCPP_BASE_LOG_D(logger, pTag, pFormat, ...)
#define EASYHTTPCPP_BASE_LOG_V(logger, pTag, pFormat, ...)
#endif

class BaseLogger {
public:
    BaseLogger();
    virtual ~BaseLogger();

    void setLoggingLevel(LogLevel level);
    LogLevel getLoggingLevel();
    void setLogWriter(LogWriter::Ptr pWriter);
    LogWriter::Ptr getLogWriter() const;
    void log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message);
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
