/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPAUTHENTICATOR_H_INCLUDED
#define EASYHTTPCPP_HTTPAUTHENTICATOR_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/Future.h"
#include "easyhttpcpp/HttpAuthenticatorCallback.h"
#include "easyhttpcpp/HttpAuthenticatorTypeDefs.h"

namespace easyhttpcpp {

/**
 * @class HttpAuthenticator HttpAuthenticator.h "easyhttpcpp/HttpAuthenticator.h"
 *
 * Represents an object that knows how to obtain authentication for a network connection.
 */
class HttpAuthenticator : public Poco::RefCountedObject {
public:
    /**
     * A "smart" pointer for Loader to facilitate reference counting based garbage collection.
     */
    typedef Poco::AutoPtr<HttpAuthenticator> Ptr;

    virtual ~HttpAuthenticator()
    {
    }

    /**
     * Fetches authorization either from cache (if present) or from server asynchronously.
     *
     * @return future object to cancel or wait for authorization.
     */
    virtual HttpAuthorizationFuturePtr getAuthorization() = 0;

    /**
     * Fetches authorization either from cache (if present) or from server asynchronously.
     *
     * @param pCallback the callback object to receive result.
     * @return future object to cancel or wait for authorization.
     */
    virtual HttpAuthorizationFuturePtr getAuthorization(HttpAuthenticatorCallback::Ptr pCallback) = 0;

    /**
     * Fetches new authorization from server asynchronously.
     *
     * @return future object to cancel or wait for authorization.
     */
    virtual HttpAuthorizationFuturePtr getNewAuthorization() = 0;

    /**
     * Fetches new authorization from server asynchronously.
     *
     * @param pCallback the callback object to receive result.
     * @return future object to cancel or wait for authorization.
     */
    virtual HttpAuthorizationFuturePtr getNewAuthorization(HttpAuthenticatorCallback::Ptr pCallback) = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPAUTHENTICATOR_H_INCLUDED */
