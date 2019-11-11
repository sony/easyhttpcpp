/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Channel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "Poco/Message.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Random.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/DefaultLogWriter.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "ColorConsoleChannelCreator.h"

namespace easyhttpcpp {
namespace common {

#define LOG_MESSAGE(tag, level, line, msg) StringUtil::format("EASYHTTPCPP_%s LogLv=%c line=%u %s", tag, level, line, msg)

static const char LoggerNamePrefix[] = "EASYHTTPCPPDefaultLogWriter";
static const char LogLevelChars[] = "VDIWE";

DefaultLogWriter::DefaultLogWriter() : m_logWriterName(createLoggerName()), m_logger(createLogger())
{
    // Set log level "trace" because log level of Logger acquired by the Poco::Logger::get is Message::PRIO_INFORMATION.
    m_logger.setLevel("trace");
}

DefaultLogWriter::~DefaultLogWriter()
{
}

const std::string& DefaultLogWriter::getName() const
{
    return m_logWriterName;
}

void DefaultLogWriter::log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message)
{
    std::string logMessage = LOG_MESSAGE(tag.c_str(), LogLevelChars[level], line, message.c_str());

    try {
        switch (level) {
            case LogLevelVerbose:
                m_logger.trace(logMessage);
                break;
            case LogLevelDebug:
                m_logger.debug(logMessage);
                break;
            case LogLevelInfo:
                m_logger.information(logMessage);
                break;
            case LogLevelWarning:
                m_logger.warning(logMessage);
                break;
            case LogLevelError:
                m_logger.error(logMessage);
                break;
            default:
                break;
        }
    } catch (const Poco::Exception&) {
        // Occurred exception at Poco's internal. Ignore because it can't do anything.
    }
}

std::string DefaultLogWriter::createLoggerName()
{
    Poco::Timestamp now;
    Poco::Random random;
    random.seed();
    return StringUtil::format("%s_%lld_%u", LoggerNamePrefix, static_cast<long long> (now.epochMicroseconds()),
            random.next());
}

Poco::Logger& DefaultLogWriter::createLogger()
{
    if (Poco::Logger::has(m_logWriterName)) {
        return Poco::Logger::get(m_logWriterName);
    } else {
        Poco::Logger& logger = Poco::Logger::get(m_logWriterName);
        logger.setChannel(createChannel());
        return logger;
    }
}

Poco::AutoPtr<Poco::FormattingChannel> DefaultLogWriter::createChannel()
{
    // When not using AutoPtr, cleanup is needed, so AutoPtr is used.
    // AutoPtr duplicate at constructor and release at destructor of FormattingChannel.
    // Same as FormattingChannel by Poco::Logger.
    Poco::AutoPtr<Poco::PatternFormatter> pFormatter = new Poco::PatternFormatter("%L%Y/%m/%d %H:%M:%S [%P:%I] %t");
    Poco::AutoPtr<Poco::Channel> pChannel = ColorConsoleChannelCreator::create();
    Poco::AutoPtr<Poco::FormattingChannel> pFCConsole = new Poco::FormattingChannel(pFormatter, pChannel);

    // Sets color brown. When the background is white, yellow is hard to see.
    // Sets color default(black) because color is gray by default.
    try {
        pFCConsole->setProperty("warningColor", "brown");
        pFCConsole->setProperty("debugColor", "default");
        pFCConsole->setProperty("traceColor", "default");
        pFCConsole->open();
    } catch (const Poco::Exception&) {
        // Occurred exception at Poco's internal. Ignore because it can't do anything.
    }

    return pFCConsole;
}

} /* namespace common */
} /* namespace easyhttpcpp */
