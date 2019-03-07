/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"

#include "CallInterceptorChain.h"
#include "HttpRequestExecutor.h"
#include "ResponseBodyStreamInternal.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "HttpRequestExecutor";
static const int MaxRetryCount = 5;

HttpRequestExecutor::HttpRequestExecutor(EasyHttpContext::Ptr pContext, Request::Ptr pRequest) : m_pContext(pContext),
        m_pUserRequest(pRequest), m_cancelled(false)
{
}

HttpRequestExecutor::~HttpRequestExecutor()
{
}

Response::Ptr HttpRequestExecutor::execute()
{
    EasyHttpContext::InterceptorList& callInterceptors = m_pContext->getCallInterceptors();
    EasyHttpContext::InterceptorList::iterator it = callInterceptors.begin();
    EasyHttpContext::InterceptorList::const_iterator itEnd = callInterceptors.end();
    if (it == itEnd) {
        return executeWithRetry(m_pUserRequest);
    } else {
        CallInterceptorChain::Ptr chain(new CallInterceptorChain(HttpRequestExecutor::Ptr(this, true), m_pUserRequest,
                it, itEnd));
        return (*it)->intercept(*chain);
    }
}

Response::Ptr HttpRequestExecutor::executeAfterIntercept(Request::Ptr pRequest)
{
    return executeWithRetry(pRequest);
}

bool HttpRequestExecutor::cancel()
{
    Poco::FastMutex::ScopedLock lock(m_cancelMutex);

    EASYHTTPCPP_LOG_D(Tag, "cancel: cancelled");
    m_cancelled = true;

    bool ret = true;
    if (m_pHttpEngine) {
        if (!m_pHttpEngine->cancel()) {
            ret = false;
        }
    }
    if (m_pUserResponse) {
        ResponseBody::Ptr pResponseBody = m_pUserResponse->getBody();
        if (pResponseBody) {
            ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
            if (pResponseBodyStream) {
                // When call the ResponseBodyStream::close, ResponseBodyStream::read will fail.
                // If you have not read to the end, not also written to the cache.
                EASYHTTPCPP_LOG_D(Tag, "cancel: close stream");
                pResponseBodyStream->close();
            }
        }
    }
    return ret;
}

bool HttpRequestExecutor::isCancelled() const
{
    return m_cancelled;
}

HttpEngine::Ptr HttpRequestExecutor::getHttpEngine()
{
    return m_pHttpEngine;
}

Response::Ptr HttpRequestExecutor::executeWithRetry(Request::Ptr pRequest)
{
    Response::Ptr pPriorResponse;
    Request::Ptr pCurrentRequest = pRequest;

    int retryCount = 0;
    do {
        {
            Poco::FastMutex::ScopedLock lock(m_cancelMutex);
            if (m_cancelled) {
                EASYHTTPCPP_LOG_D(Tag, "executeWithRetry: request is cancelled before create HttpEngine.");
                throw HttpExecutionException("http request is cancelled.");
            }
            m_pHttpEngine = new HttpEngine(m_pContext, pCurrentRequest, pPriorResponse);
        }
        Response::Ptr pUserResponse = m_pHttpEngine->execute();

        // check retry
        Request::Ptr pRetryRequest = HttpEngine::getRetryRequest(pUserResponse);
        if (pRetryRequest) {
            m_pHttpEngine->readAllResponseBodyForCache(pUserResponse);
            pPriorResponse = pUserResponse;
            pCurrentRequest = pRetryRequest;
            retryCount++;
            continue;
        }

        {
            Poco::FastMutex::ScopedLock lock(m_cancelMutex);
            m_pUserResponse = pUserResponse;
        }

        return m_pUserResponse;

    } while (retryCount <= MaxRetryCount);

    EASYHTTPCPP_LOG_D(Tag, "retry count over %d times.", MaxRetryCount);
    throw HttpExecutionException(StringUtil::format("too many retry request. %d times.", MaxRetryCount));
}

} /* namespace easyhttpcpp */
