/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CommonException.h"
#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {

EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(CommonException, CoreException, ExceptionConstants::SubGroupCode::Common)

EASYHTTPCPP_IMPLEMENT_EXCEPTION(PocoException, CommonException, 0)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(StdException, CommonException, 1)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(FutureIllegalStateException, CommonException, 2)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(FutureCancellationException, CommonException, 3)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(FutureExecutionException, CommonException, 4)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(FutureTimeoutException, CommonException, 5)

} /* namespace common */
} /* namespace easyhttpcpp */
