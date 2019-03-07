/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSECALLBACK_H_INCLUDED
#define EASYHTTPCPP_RESPONSECALLBACK_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Response.h"

namespace easyhttpcpp {

/**
 * @class ResponseCallback ResponseCallback.h "easyhttpcpp/ResponseCallback.h"
 *
 * Completion callback class for Call::executeAsync.
 */
class ResponseCallback : public Poco::RefCountedObject {
public:
    /**
     * A "smart" pointer to facilitate reference counting based garbage collection.
     */
    typedef Poco::AutoPtr<ResponseCallback> Ptr;

    virtual ~ResponseCallback()
    {
    }

    /**
     * Called when the Http response was returned by the remote server.
     *
     * @param pResponse Response object that returned by remote server.
     */
    virtual void onResponse(Response::Ptr pResponse) = 0;

    /**
     * Called when the exception occurred in executing Call::executeAsync.
     * 
     * @param pWhat the exception that occurred in executing Call::executeAsync.
     */
    virtual void onFailure(HttpException::Ptr pWhat) = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSECALLBACK_H_INCLUDED */
