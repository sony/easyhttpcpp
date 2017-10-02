/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/DefaultLogWriter.h"
#include "easyhttpcpp/common/BaseLogger.h"
#include "easyhttpcpp/common/LogLevel.h"
#include "easyhttpcpp/common/LogWriter.h"

namespace easyhttpcpp {
namespace common {

#ifndef NDEBUG
const LogLevel BaseLogger::DefaultLogLevel = LogLevelVerbose;
#else
const LogLevel BaseLogger::DefaultLogLevel = LogLevelWarning;
#endif

BaseLogger::BaseLogger() : m_level(DefaultLogLevel), m_pWriter(new DefaultLogWriter())
{
}

BaseLogger::~BaseLogger()
{
}

void BaseLogger::setLoggingLevel(LogLevel level)
{
    m_level = level;
}

LogLevel BaseLogger::getLoggingLevel()
{
    return m_level;
}

void BaseLogger::setLogWriter(LogWriter::Ptr pWriter)
{
    if (pWriter) {
        m_pWriter = pWriter;
    }
}

LogWriter::Ptr BaseLogger::getLogWriter() const
{
    return m_pWriter;
}

void BaseLogger::log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message)
{
    if (isLoggable(level)) {
        // When setLogWriter() has been executed in the log() run, LogWriter will be automatic release.
        // In order to avoid the automatic release, use to the local copy of the LogWriter.
        LogWriter::Ptr pLocalWriter = m_pWriter;
        pLocalWriter->log(tag, level, line, message);
    }
}

void BaseLogger::resetToDefaults()
{
    m_level = DefaultLogLevel;
    m_pWriter = new DefaultLogWriter();
}

bool BaseLogger::isLoggable(LogLevel level)
{
    return (level >= m_level);
}

} /* namespace common */
} /* namespace easyhttpcpp */
