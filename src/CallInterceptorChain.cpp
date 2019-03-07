/*
 * Copyright 2017 Sony Corporation
 */

#include "CallInterceptorChain.h"

namespace easyhttpcpp {

CallInterceptorChain::CallInterceptorChain(HttpRequestExecutor::Ptr pRequestExecutor, Request::Ptr pRequest,
        EasyHttpContext::InterceptorList::iterator& currentIterator,
        EasyHttpContext::InterceptorList::const_iterator& endIterator) :
        m_pRequestExecutor(pRequestExecutor), m_pRequest(pRequest), m_currentIterator(currentIterator),
        m_endIterator(endIterator)
{
}

CallInterceptorChain::~CallInterceptorChain()
{
}

Request::Ptr CallInterceptorChain::getRequest() const
{
    return m_pRequest;
}

Connection::Ptr CallInterceptorChain::getConnection() const
{
    return NULL;
}

Response::Ptr CallInterceptorChain::proceed(Request::Ptr pRequest)
{
    m_currentIterator++;
    if (m_currentIterator != m_endIterator) {
        CallInterceptorChain::Ptr chain(new CallInterceptorChain(m_pRequestExecutor, pRequest, m_currentIterator,
                m_endIterator));
        return (*m_currentIterator)->intercept(*chain);
    } else {
        return m_pRequestExecutor->executeAfterIntercept(pRequest);
    }
}

} /* namespace easyhttpcpp */
