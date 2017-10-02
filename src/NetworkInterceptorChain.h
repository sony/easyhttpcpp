/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_NETWORKINTERCEPTORCHAIN_H_INCLUDED
#define EASYHTTPCPP_NETWORKINTERCEPTORCHAIN_H_INCLUDED

#include "easyhttpcpp/Interceptor.h"

#include "EasyHttpContext.h"
#include "HttpEngine.h"

namespace easyhttpcpp {

class NetworkInterceptorChain : public Interceptor::Chain {
public:
    NetworkInterceptorChain(HttpEngine& httpEngine, Request::Ptr pRequest,
            EasyHttpContext::InterceptorList::iterator& currentIterator,
            EasyHttpContext::InterceptorList::const_iterator& endIterator);
    virtual ~NetworkInterceptorChain();
    virtual Request::Ptr getRequest() const;
    virtual Connection::Ptr getConnection() const;
    virtual Response::Ptr proceed(Request::Ptr pRequest);

private:
    HttpEngine& m_httpEngine;
    Request::Ptr m_pRequest;
    EasyHttpContext::InterceptorList::iterator& m_currentIterator;
    EasyHttpContext::InterceptorList::const_iterator& m_endIterator;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_NETWORKINTERCEPTORCHAIN_H_INCLUDED */
