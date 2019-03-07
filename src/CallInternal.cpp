/*
 * Copyright 2017 Sony Corporation
 */

#include <istream>
#include <ostream>

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "CallInternal.h"
#include "HttpAsyncExecutionTask.h"

namespace easyhttpcpp {

static const std::string Tag = "CallInternal";

CallInternal::CallInternal(EasyHttpContext::Ptr pContext, Request::Ptr pRequest) : m_pContext(pContext),
        m_pUserRequest(pRequest), m_executed(false), m_cancelled(false)
{
}

CallInternal::~CallInternal()
{
}

Response::Ptr CallInternal::execute()
{
    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);

        if (m_executed) {
            EASYHTTPCPP_LOG_D(Tag, "Can not execute because already executed.");
            throw HttpIllegalStateException("Can not execute because already executed.");
        }

        m_executed = true;

        m_pRequestExecutor = new HttpRequestExecutor(m_pContext, m_pUserRequest);
        if (m_cancelled) {
            m_pRequestExecutor->cancel();
        }
    }

    return m_pRequestExecutor->execute();
}

void CallInternal::executeAsync(ResponseCallback::Ptr pResponseCallback)
{
    HttpAsyncExecutionTask::Ptr pAsyncExecutionTask;
    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);

        if (m_executed) {
            EASYHTTPCPP_LOG_D(Tag, "Can not execute because already executed.");
            throw HttpIllegalStateException("Can not execute because already executed.");
        }

        if (!pResponseCallback) {
            EASYHTTPCPP_LOG_D(Tag, "ResponseCallback is NULL.");
            throw HttpIllegalArgumentException("ResponseCallback can not be NULL.");
        }

        m_executed = true;

        m_pRequestExecutor = new HttpRequestExecutor(m_pContext, m_pUserRequest);
        pAsyncExecutionTask = new HttpAsyncExecutionTask(m_pContext, m_pRequestExecutor,
                pResponseCallback);

        if (m_cancelled) {
            m_pRequestExecutor->cancel();
        }
    }

    m_pContext->getHttpExecutionTaskManager()->start(pAsyncExecutionTask);
}

bool CallInternal::isExecuted() const
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    return m_executed;
}

Request::Ptr CallInternal::getRequest() const
{
    return m_pUserRequest;
}

bool CallInternal::cancel()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    m_cancelled = true;
    if (m_pRequestExecutor) {
        return m_pRequestExecutor->cancel();
    } else {
        return true;
    }
}

bool CallInternal::isCancelled() const
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (m_pRequestExecutor) {
        return m_pRequestExecutor->isCancelled();
    } else {
        return m_cancelled;
    }
}

HttpEngine::Ptr CallInternal::getHttpEngine()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (m_pRequestExecutor) {
        return m_pRequestExecutor->getHttpEngine();
    } else {
        return NULL;
    }
}

} /* namespace easyhttpcpp */
