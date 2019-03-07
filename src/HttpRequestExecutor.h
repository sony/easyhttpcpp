/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPREQUESTEXECUTOR_H_INCLUDED
#define EASYHTTPCPP_HTTPREQUESTEXECUTOR_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

#include "EasyHttpContext.h"
#include "HttpEngine.h"

namespace easyhttpcpp {

class HttpRequestExecutor : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpRequestExecutor> Ptr;

    HttpRequestExecutor(EasyHttpContext::Ptr pContext, Request::Ptr pRequest);
    virtual ~HttpRequestExecutor();

    Response::Ptr execute();
    Response::Ptr executeAfterIntercept(Request::Ptr pRequest);
    bool cancel();
    bool isCancelled() const;
    HttpEngine::Ptr getHttpEngine();

private:
    HttpRequestExecutor();
    Response::Ptr executeWithRetry(Request::Ptr pRequest);

    EasyHttpContext::Ptr m_pContext;
    Request::Ptr m_pUserRequest;
    Response::Ptr m_pUserResponse;
    HttpEngine::Ptr m_pHttpEngine;
    bool m_cancelled;
    Poco::FastMutex m_cancelMutex;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPREQUESTEXECUTOR_H_INCLUDED */
