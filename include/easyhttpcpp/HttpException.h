/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"
#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(EASYHTTPCPP_HTTP_API, HttpException, easyhttpcpp::common::CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpIllegalArgumentException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpIllegalStateException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpExecutionException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpTimeoutException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpSslException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_HTTP_API, HttpConnectionRetryException, HttpException)

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED */
