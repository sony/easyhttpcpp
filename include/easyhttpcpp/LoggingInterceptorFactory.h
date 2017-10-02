/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_LOGGINGINTERCEPTORFACTORY_H_INCLUDED
#define EASYHTTPCPP_LOGGINGINTERCEPTORFACTORY_H_INCLUDED

#include "easyhttpcpp/LoggingInterceptor.h"

namespace easyhttpcpp {

class LoggingInterceptorFactory {
public:
    LoggingInterceptorFactory();
    virtual ~LoggingInterceptorFactory();

    virtual LoggingInterceptor::Ptr interceptor(LoggingInterceptorType interceptorType);
private:
};

} /* namespace easyhttpcpp */


#endif /* EASYHTTPCPP_LOGGINGINTERCEPTORFACTORY_H_INCLUDED */
