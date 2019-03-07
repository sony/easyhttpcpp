/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"

namespace easyhttpcpp {
namespace executorservice {

using easyhttpcpp::common::ExceptionConstants;

EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(ExecutorServiceException, CoreException,
        ExceptionConstants::SubGroupCode::ExecutorService)

EASYHTTPCPP_IMPLEMENT_EXCEPTION(ExecutorServiceIllegalArgumentException, ExecutorServiceException, 0)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(ExecutorServiceIllegalStateException, ExecutorServiceException, 1)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(ExecutorServiceExecutionException, ExecutorServiceException, 2)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(ExecutorServiceTooManyRequestsException, ExecutorServiceException, 3)

} /* namespace executorservice */
} /* namespace easyhttpcpp */

