/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_SSLCONTEXTCREATORIMPL_H_INCLUDED
#define EASYHTTPCPP_SSLCONTEXTCREATORIMPL_H_INCLUDED

#include "Poco/Net/Context.h"

#include "EasyHttpContext.h"

namespace easyhttpcpp {

class SslContextCreatorImpl {
public:
    static Poco::Net::Context::Ptr createContext(EasyHttpContext::Ptr pContext);
private:
    SslContextCreatorImpl();
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_SSLCONTEXTCREATORIMPL_H_INCLUDED */
