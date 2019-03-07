/*
 * Copyright 2018 Sony Corporation
 */

#include "easyhttpcpp/common/StringUtil.h"
#include "TestLogger.h"

using easyhttpcpp::common::LogLevel;
using easyhttpcpp::common::LogLevelVerbose;
using easyhttpcpp::common::LogWriter;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace testutil {
namespace {

class TestTagPrefixingLogWriter : public LogWriter {
public:

    TestTagPrefixingLogWriter(LogWriter::Ptr pDefaultWriter) : m_pDefaultWriter(pDefaultWriter)
    {
    }

    virtual ~TestTagPrefixingLogWriter()
    {
    }

    virtual const std::string& getName() const
    {
        return m_pDefaultWriter->getName();
    }

    virtual void log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message)
    {
        m_pDefaultWriter->log(StringUtil::format("TEST_%s", tag.c_str()), level, line, message);
    }

private:
    LogWriter::Ptr m_pDefaultWriter;
};

} /* namespace */

bool TestLogger::s_terminated = false;

TestLogger* TestLogger::getInstance()
{
    static Poco::SingletonHolder<TestLogger> s_singleton;

    if (s_terminated) {
        return NULL;
    } else {
        return s_singleton.get();
    }
}

TestLogger::TestLogger()
{
    setLogWriter(new TestTagPrefixingLogWriter(getLogWriter()));
    setLoggingLevel(LogLevelVerbose);
}

TestLogger::~TestLogger()
{
    s_terminated = true;
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
