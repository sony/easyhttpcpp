/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_SSLCONTEXTCREATOR_H_INCLUDED
#define EASYHTTPCPP_SSLCONTEXTCREATOR_H_INCLUDED

#include "Poco/Net/Context.h"

#include "EasyHttpContext.h"

namespace easyhttpcpp {

class SslContextCreator {
public:
    static Poco::Net::Context::Ptr createContext(EasyHttpContext::Ptr pContext);
private:
    SslContextCreator();
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_SSLCONTEXTCREATOR_H_INCLUDED */
