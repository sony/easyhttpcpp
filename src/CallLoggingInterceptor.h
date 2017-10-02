/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CALLLOGGINGINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_CALLLOGGINGINTERCEPTOR_H_INCLUDED

#include "easyhttpcpp/LoggingInterceptor.h"

namespace easyhttpcpp {

class CallLoggingInterceptor : public LoggingInterceptor {
public:
    CallLoggingInterceptor();
    virtual ~CallLoggingInterceptor();

    virtual Response::Ptr intercept(Chain& chain);
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CALLLOGGINGINTERCEPTOR_H_INCLUDED */
