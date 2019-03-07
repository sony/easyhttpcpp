/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpAsyncExecutionTask.h"

namespace easyhttpcpp {

static const std::string Tag = "HttpAsyncExecutionTask";

HttpAsyncExecutionTask::HttpAsyncExecutionTask(EasyHttpContext::Ptr pContext,
        HttpRequestExecutor::Ptr pRequestExecutor, ResponseCallback::Ptr pResponseCallback) :
        m_pContext(pContext), m_pRequestExecutor(pRequestExecutor), m_pResponseCallback(pResponseCallback)
{
}

HttpAsyncExecutionTask::~HttpAsyncExecutionTask()
{
}

void HttpAsyncExecutionTask::runTask()
{
    Response::Ptr pResponse;
    try {
        pResponse = m_pRequestExecutor->execute();
    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error while executing Http asynchronous request. Details: %s", e.getMessage().c_str());
        notifyCompletion(e.clone(), NULL);
        return;
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Unexpected error while executing Http asynchronous request. Details: %s", e.what());
        HttpExecutionException::Ptr pWhat = new HttpExecutionException(
                "Unexpected error while executing Http asynchronous request. Check getCause() for details.", e);
        notifyCompletion(pWhat, NULL);
        return;
    }

    EASYHTTPCPP_LOG_D(Tag, "Http asynchronous request succeeded.");
    notifyCompletion(NULL, pResponse);
}

bool HttpAsyncExecutionTask::cancel(bool mayInterruptIfRunning)
{
    return m_pRequestExecutor->cancel();
}

bool HttpAsyncExecutionTask::isCancelled() const
{
    return m_pRequestExecutor->isCancelled();
}

void HttpAsyncExecutionTask::notifyCompletion(HttpException::Ptr pWhat, Response::Ptr pResponse)
{
    HttpExecutionTaskManager::Ptr pExecutionTaskManager = m_pContext->getHttpExecutionTaskManager();
    pExecutionTaskManager->onComplete(HttpExecutionTask::Ptr(this, true));

    if (m_pResponseCallback) {
        if (pWhat) {
            m_pResponseCallback->onFailure(pWhat);
        } else {
            m_pResponseCallback->onResponse(pResponse);
        }
    }
}

} /* namespace easyhttpcpp */
