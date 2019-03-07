/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED

#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Interceptor.h"

namespace easyhttpcpp {

enum LoggingInterceptorType {
    LoggingInterceptorTypeCall,
    LoggingInterceptorTypeNetwork
};

class EASYHTTPCPP_HTTP_API LoggingInterceptor : public Interceptor {
public:
    virtual ~LoggingInterceptor()
    {
    }
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_LOGGINGINTERCEPTOR_H_INCLUDED */
