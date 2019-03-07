/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CALL_H_INCLUDED
#define EASYHTTPCPP_CALL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseCallback.h"

namespace easyhttpcpp {

/**
 * @brief A Call execute Http Request.
 */
class Call : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Call> Ptr;

    /**
     * @brief destructor
     */
    virtual ~Call()
    {
    }

    /**
     * @brief Execute Http Request synchronously.
     * 
     * Invokes the request immediately, and blocks until the response can be processed or is in error.
     * @return Response
     * @exception HttpIllegalStateException
     * @exception HttpIllegalArgumentException
     * @exception HttpExecutionException
     * @exception HttpTimeoutException
     * @exception HttpSslException
     */
    virtual Response::Ptr execute() = 0;

    /**
     * @brief Execute Http Request asynchronously.
     * 
     * This method returns as soon as it accepts the request.
     * The request is executed asynchronously, and when response is received, callback is invoked.
     * @param pResponseCallback
     * @exception HttpIllegalStateException
     * @exception HttpIllegalArgumentException
     */
    virtual void executeAsync(ResponseCallback::Ptr pResponseCallback) = 0;

    /**
     * @brief Returns true if this call has been either executed.
     * @return true if already executed.
     * @deprecated
     */
#ifdef _WIN32
    _declspec(deprecated) virtual bool isExecuted() const = 0;
#else
    __attribute__ ((deprecated)) virtual bool isExecuted() const = 0;
#endif

    /**
     * @brief Get Request.
     * @return Request
     */
    virtual Request::Ptr getRequest() const = 0;

    virtual bool cancel() = 0;

    virtual bool isCancelled() const = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CALL_H_INCLUDED */
