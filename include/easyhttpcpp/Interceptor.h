/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_INTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_INTERCEPTOR_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Connection.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

namespace easyhttpcpp {

class Interceptor : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Interceptor> Ptr;

    class Chain;

    virtual ~Interceptor()
    {
    }

    virtual Response::Ptr intercept(Chain& chain) = 0;

public:

    class Chain : public Poco::RefCountedObject {
    public:
        typedef Poco::AutoPtr<Chain> Ptr;

        virtual ~Chain()
        {
        }

        virtual Request::Ptr getRequest() const = 0;

        virtual Connection::Ptr getConnection() const = 0;

        virtual Response::Ptr proceed(Request::Ptr pRequest) = 0;
    };

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_INTERCEPTOR_H_INCLUDED */
