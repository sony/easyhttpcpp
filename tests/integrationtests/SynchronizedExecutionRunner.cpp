/*
 * Copyright 2017 Sony Corporation
 */

#include "TestLogger.h"

#include "SynchronizedExecutionRunner.h"

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "SynchronizedExecutionRunner";
static const int TestFailureTimeout = 10 * 1000; // milliseconds

SynchronizedExecutionRunner::SynchronizedExecutionRunner() : m_succeeded(false)
{
}

SynchronizedExecutionRunner::~SynchronizedExecutionRunner()
{
}

void SynchronizedExecutionRunner::run()
{
    m_succeeded = execute();
    setToFinish();
}

bool SynchronizedExecutionRunner::isSuccess()
{
    return m_succeeded;
}

bool SynchronizedExecutionRunner::waitToReady()
{
    if (!m_readyEvent.tryWait(TestFailureTimeout)) {
        EASYHTTPCPP_TESTLOG_E(Tag, "waitToReady is time out");
        return false;
    }
    return true;
}

void SynchronizedExecutionRunner::setToStart()
{
    m_startEvent.set();
}

bool SynchronizedExecutionRunner::waitToFinish()
{
    if (!m_finishEvent.tryWait(TestFailureTimeout)) {
        EASYHTTPCPP_TESTLOG_E(Tag, "waitToFinish is time out");
        return false;
    }
    return true;
}

void SynchronizedExecutionRunner::setToReady()
{
    m_readyEvent.set();
}

bool SynchronizedExecutionRunner::waitToStart()
{
    if (!m_startEvent.tryWait(TestFailureTimeout)) {
        EASYHTTPCPP_TESTLOG_E(Tag, "waitToStart is time out");
        return false;
    }
    return true;
}

void SynchronizedExecutionRunner::setToFinish()
{
    m_finishEvent.set();
}

} /* namespace test */
} /* namespace easyhttpcpp */
