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
#include "HttpRequestExecutor.h"

namespace easyhttpcpp {

class CallInternal : public Call {
public:

    CallInternal(EasyHttpContext::Ptr pContext, Request::Ptr pRequest);
    virtual ~CallInternal();
    virtual Response::Ptr execute();
    virtual void executeAsync(ResponseCallback::Ptr pResponseCallback);
#ifdef _WIN32
    _declspec(deprecated) virtual bool isExecuted() const;
#else
    __attribute__ ((deprecated)) virtual bool isExecuted() const;
#endif
    virtual Request::Ptr getRequest() const;
    virtual bool cancel();
    virtual bool isCancelled() const;

    HttpEngine::Ptr getHttpEngine();
private:
    EasyHttpContext::Ptr m_pContext;
    Request::Ptr m_pUserRequest;
    mutable Poco::FastMutex m_instanceMutex;
    bool m_executed;
    bool m_cancelled;
    HttpRequestExecutor::Ptr m_pRequestExecutor;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CALLINTERNAL_H_INCLUDED */
