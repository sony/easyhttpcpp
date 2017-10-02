/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/LoggingInterceptorFactory.h"

#include "CallLoggingInterceptor.h"
#include "NetworkLoggingInterceptor.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

LoggingInterceptorFactory::LoggingInterceptorFactory()
{
}

LoggingInterceptorFactory::~LoggingInterceptorFactory()
{
}

LoggingInterceptor::Ptr LoggingInterceptorFactory::interceptor(LoggingInterceptorType interceptorType)
{
    switch (interceptorType) {
        case LoggingInterceptorTypeCall:
            return new CallLoggingInterceptor();
        case LoggingInterceptorTypeNetwork:
            return new NetworkLoggingInterceptor();
        default:
            throw HttpIllegalArgumentException(StringUtil::format("Unknown logging interceptor type: [%d]",
                    interceptorType));
    }
}

} /* namespace easyhttpcpp */

