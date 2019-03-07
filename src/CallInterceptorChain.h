/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/Interceptor.h"

#include "EasyHttpContext.h"
#include "HttpRequestExecutor.h"

#ifndef EASYHTTPCPP_APPLICATIONINTERCEPTORCHAIN_H_INCLUDED
#define EASYHTTPCPP_APPLICATIONINTERCEPTORCHAIN_H_INCLUDED

namespace easyhttpcpp {

class CallInterceptorChain : public Interceptor::Chain {
public:
    CallInterceptorChain(HttpRequestExecutor::Ptr pRequestExecutior, Request::Ptr pRequest,
            EasyHttpContext::InterceptorList::iterator& currentIterator,
            EasyHttpContext::InterceptorList::const_iterator& endIterator);
    virtual ~CallInterceptorChain();
    virtual Request::Ptr getRequest() const;
    virtual Connection::Ptr getConnection() const;
    virtual Response::Ptr proceed(Request::Ptr pRequest);

private:
    HttpRequestExecutor::Ptr m_pRequestExecutor;
    Request::Ptr m_pRequest;
    EasyHttpContext::InterceptorList::iterator& m_currentIterator;
    EasyHttpContext::InterceptorList::const_iterator& m_endIterator;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_APPLICATIONINTERCEPTORCHAIN_H_INCLUDED */
