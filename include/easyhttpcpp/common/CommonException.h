/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CommonExports.h"
#include "easyhttpcpp/common/CoreException.h"

namespace easyhttpcpp {
namespace common {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(EASYHTTPCPP_COMMON_API, CommonException, CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, PocoException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, StdException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, FutureIllegalStateException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, FutureCancellationException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, FutureExecutionException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_COMMON_API, FutureTimeoutException, CommonException)

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED */
