/*
 * Copyright 2017 Sony Corporation
 */

#include "NetworkInterceptorChain.h"

namespace easyhttpcpp {

NetworkInterceptorChain::NetworkInterceptorChain(HttpEngine& httpEngine, Request::Ptr pRequest,
        EasyHttpContext::InterceptorList::iterator& currentIterator,
        EasyHttpContext::InterceptorList::const_iterator& endIterator) :
        m_httpEngine(httpEngine), m_pRequest(pRequest), m_currentIterator(currentIterator), m_endIterator(endIterator)
{
}

NetworkInterceptorChain::~NetworkInterceptorChain()
{
}

Request::Ptr NetworkInterceptorChain::getRequest() const
{
    return m_pRequest;
}

Connection::Ptr NetworkInterceptorChain::getConnection() const
{
    return m_httpEngine.getConnection();
}

Response::Ptr NetworkInterceptorChain::proceed(Request::Ptr pRequest)
{
    Response::Ptr pResponse;
    m_currentIterator++;
    if (m_currentIterator != m_endIterator) {
        NetworkInterceptorChain* pNetworkInterpectorChain =  new NetworkInterceptorChain(m_httpEngine, pRequest,
                m_currentIterator, m_endIterator);
        Interceptor::Chain::Ptr pChain = pNetworkInterpectorChain;
        pResponse = (*m_currentIterator)->intercept(*pNetworkInterpectorChain);
    } else {
        pResponse = m_httpEngine.sendRequestAndReceiveResponseWithRetryByConnection(pRequest);
    }
    return pResponse;
}

} /* namespace easyhttpcpp */
