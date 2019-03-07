/*
 * Copyright 2017 Sony Corporation
 */

#include "HttpTestResponseCallback.h"

namespace easyhttpcpp {
namespace test {

static const int TestFailureTimeout = 10 * 1000; // milliseconds

HttpTestResponseCallback::HttpTestResponseCallback()
{
    m_completionTime = Poco::Timestamp::TIMEVAL_MAX;
}

void HttpTestResponseCallback::onResponse(Response::Ptr pResponse)
{
    m_pResponse = pResponse;
    m_completionTime.update();
    m_completionEvent.set();
}

void HttpTestResponseCallback::onFailure(HttpException::Ptr pWhat)
{
    m_pWhat = pWhat;
    m_completionTime.update();
    m_completionEvent.set();
}

Response::Ptr HttpTestResponseCallback::getResponse()
{
    return m_pResponse;
}

HttpException::Ptr HttpTestResponseCallback::getWhat()
{
    return m_pWhat;
}

bool HttpTestResponseCallback::waitCompletion()
{
    return m_completionEvent.tryWait(TestFailureTimeout);
}

bool HttpTestResponseCallback::waitCompletion(long timeoutMilliSec)
{
    return m_completionEvent.tryWait(timeoutMilliSec);
}

Poco::Timestamp HttpTestResponseCallback::getCompletionTime() const
{
    return m_completionTime;
}

} /* namespace test */
} /* namespace easyhttpcpp */
