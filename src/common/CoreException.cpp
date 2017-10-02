/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreException.h"
#include "easyhttpcpp/common/ExceptionConstants.h"

namespace easyhttpcpp {
namespace common {

EASYHTTPCPP_IMPLEMENT_EXCEPTION_GROUP(CoreException, ExceptionConstants::GroupCode::Core)

} /* namespace common */
} /* namespace easyhttpcpp */
