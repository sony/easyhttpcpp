/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CALLINTERNAL_H_INCLUDED
#define EASYHTTPCPP_CALLINTERNAL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

#include "EasyHttpContext.h"
#include "HttpEngine.h"

namespace easyhttpcpp {

class CallInternal : public Call {
public:

    CallInternal(EasyHttpContext::Ptr pContext, Request::Ptr pRequest);
    virtual ~CallInternal();
    virtual Response::Ptr execute();
    virtual bool isExecuted() const;
    virtual Request::Ptr getRequest() const;
    virtual bool cancel();
    virtual bool isCancelled() const;

    Response::Ptr executeAfterIntercept(Request::Ptr pRequest);
    HttpEngine::Ptr getHttpEngine();
private:
    Response::Ptr executeWithRetry(Request::Ptr pRequest);

    EasyHttpContext::Ptr m_pContext;
    Request::Ptr m_pUserRequest;
    Response::Ptr m_pUserResponse;
    bool m_executed;
    HttpEngine::Ptr m_pHttpEngine;
    Poco::FastMutex m_cancelMutex;
    bool m_cancelled;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CALLINTERNAL_H_INCLUDED */
