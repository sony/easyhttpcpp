/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED

#include "easyhttpcpp/Interceptor.h"

namespace easyhttpcpp {

enum LoggingInterceptorType {
    LoggingInterceptorTypeCall,
    LoggingInterceptorTypeNetwork
};

class LoggingInterceptor : public Interceptor {
public:
    virtual ~LoggingInterceptor()
    {
    }
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED */
