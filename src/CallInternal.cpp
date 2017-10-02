/*
 * Copyright 2017 Sony Corporation
 */

#include <istream>
#include <ostream>

#include "Poco/Net/NetSSL.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"

#include "CallInterceptorChain.h"
#include "CallInternal.h"
#include "ResponseBodyStreamInternal.h"

using easyhttpcpp::common::BaseException;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "CallInternal";
static const int MaxRetryCount = 5;

CallInternal::CallInternal(EasyHttpContext::Ptr pContext, Request::Ptr pRequest) : m_pContext(pContext),
        m_pUserRequest(pRequest), m_executed(false), m_cancelled(false)
{
}

CallInternal::~CallInternal()
{
}

Response::Ptr CallInternal::execute()
{
    if (m_executed) {
        std::string message = "can not execute because already executed.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalStateException(message);
    }

    m_executed = true;

    EasyHttpContext::InterceptorList& callInterceptors = m_pContext->getCallInterceptors();
    EasyHttpContext::InterceptorList::iterator it = callInterceptors.begin();
    EasyHttpContext::InterceptorList::const_iterator itEnd = callInterceptors.end();
    if (it == itEnd) {
        return executeWithRetry(m_pUserRequest);
    } else {
        CallInterceptorChain::Ptr chain(new CallInterceptorChain(*this, m_pUserRequest, it, itEnd));
        return (*it)->intercept(*chain);
    }
}

bool CallInternal::isExecuted() const
{
    return m_executed;
}

Request::Ptr CallInternal::getRequest() const
{
    return m_pUserRequest;
}

bool CallInternal::cancel()
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

bool CallInternal::isCancelled() const
{
    return m_cancelled;
}

Response::Ptr CallInternal::executeAfterIntercept(Request::Ptr pRequest)
{
    return executeWithRetry(pRequest);
}

HttpEngine::Ptr CallInternal::getHttpEngine()
{
    return m_pHttpEngine;
}

Response::Ptr CallInternal::executeWithRetry(Request::Ptr pRequest)
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

        m_pUserResponse = pUserResponse;

        return m_pUserResponse;

    } while (retryCount <= MaxRetryCount);

    EASYHTTPCPP_LOG_D(Tag, "retry count over %d times.", MaxRetryCount);
    throw HttpExecutionException(StringUtil::format("too many retry request. %d times.", MaxRetryCount));
}

} /* namespace easyhttpcpp */
