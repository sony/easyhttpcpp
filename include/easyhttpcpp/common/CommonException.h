/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"

namespace easyhttpcpp {
namespace common {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(CommonException, CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(PocoException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(StdException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(FutureIllegalStateException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(FutureCancellationException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(FutureExecutionException, CommonException)

EASYHTTPCPP_DECLARE_EXCEPTION(FutureTimeoutException, CommonException)

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COMMONEXCEPTION_H_INCLUDED */
