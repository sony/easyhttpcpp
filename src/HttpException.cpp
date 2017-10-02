/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"

namespace easyhttpcpp {

using easyhttpcpp::common::ExceptionConstants;

EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(HttpException, CoreException, ExceptionConstants::SubGroupCode::Http)

EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpIllegalArgumentException, HttpException, 0)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpIllegalStateException, HttpException, 1)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpExecutionException, HttpException, 2)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpTimeoutException, HttpException, 3)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpSslException, HttpException, 4)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(HttpConnectionRetryException, HttpException, 5)

} /* namespace easyhttpcpp */
