/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_NETWORKLOGGINGINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_NETWORKLOGGINGINTERCEPTOR_H_INCLUDED

#include "easyhttpcpp/LoggingInterceptor.h"

namespace easyhttpcpp {

class NetworkLoggingInterceptor : public LoggingInterceptor {
public:
    NetworkLoggingInterceptor();
    virtual ~NetworkLoggingInterceptor();

    virtual Response::Ptr intercept(Chain& chain);
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_NETWORKLOGGINGINTERCEPTOR_H_INCLUDED */
