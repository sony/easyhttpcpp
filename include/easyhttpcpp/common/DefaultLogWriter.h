/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_DEFAULTLOGWRITER_H_INCLUDED
#define EASYHTTPCPP_COMMON_DEFAULTLOGWRITER_H_INCLUDED

#include "Poco/Logger.h"
#include "Poco/FormattingChannel.h"

#include "easyhttpcpp/common/LogWriter.h"

namespace easyhttpcpp {
namespace common {

class DefaultLogWriter : public LogWriter {
public:
    DefaultLogWriter();
    virtual ~DefaultLogWriter();
    virtual const std::string& getName() const;
    virtual void log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message);

private:
    std::string createLoggerName();
    Poco::Logger& createLogger();
    Poco::AutoPtr<Poco::FormattingChannel> createChannel();

    std::string m_logWriterName;
    Poco::Logger& m_logger;

};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_DEFAULTLOGWRITER_H_INCLUDED */
