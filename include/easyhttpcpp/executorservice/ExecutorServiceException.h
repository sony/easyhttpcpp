/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"
#include "easyhttpcpp/executorservice/ExecutorServiceExports.h"

namespace easyhttpcpp {
namespace executorservice {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(EASYHTTPCPP_EXECUTORSERVICE_API, ExecutorServiceException, easyhttpcpp::common::CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_EXECUTORSERVICE_API, ExecutorServiceIllegalArgumentException, ExecutorServiceException)
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_EXECUTORSERVICE_API, ExecutorServiceIllegalStateException, ExecutorServiceException)
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_EXECUTORSERVICE_API, ExecutorServiceExecutionException, ExecutorServiceException)
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_EXECUTORSERVICE_API, ExecutorServiceTooManyRequestsException, ExecutorServiceException)

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXCEPTION_H_INCLUDED */
