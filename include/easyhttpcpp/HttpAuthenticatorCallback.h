/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPAUTHENTICATORCALLBACK_H_INCLUDED
#define EASYHTTPCPP_HTTPAUTHENTICATORCALLBACK_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/HttpException.h"

namespace easyhttpcpp {

/**
 * @class HttpAuthenticatorCallback HttpAuthenticatorCallback.h "easyhttpcpp/HttpAuthenticatorCallback.h"
 *
 * Completion callback class for HttpAuthenticator.
 */
class HttpAuthenticatorCallback : public Poco::RefCountedObject {
public:
    /**
     * A "smart" pointer for Loader to facilitate reference counting based garbage collection.
     */
    typedef Poco::AutoPtr<HttpAuthenticatorCallback> Ptr;

    virtual ~HttpAuthenticatorCallback()
    {
    }

    /**
     * Called when HttpAuthenticator completes operation.
     *
     * @param pWhat the exception occurred or null if the operation succeeded.
     * @param authorization the authorization or empty if an error occurred.
     */
    virtual void onComplete(HttpException::Ptr pWhat, const std::string& authorization) = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPAUTHENTICATORCALLBACK_H_INCLUDED */
