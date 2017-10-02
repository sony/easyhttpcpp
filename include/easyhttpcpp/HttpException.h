/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"

namespace easyhttpcpp {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(HttpException, easyhttpcpp::common::CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpIllegalArgumentException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpIllegalStateException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpExecutionException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpTimeoutException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpSslException, HttpException)

EASYHTTPCPP_DECLARE_EXCEPTION(HttpConnectionRetryException, HttpException)

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPEXCEPTION_H_INCLUDED */
