/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/messagedigest/MessageDigestException.h"

using easyhttpcpp::common::CoreException;
using easyhttpcpp::common::ExceptionConstants;

namespace easyhttpcpp {
namespace messagedigest {

EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(MessageDigestException, CoreException, ExceptionConstants::SubGroupCode::MessageDigest)

EASYHTTPCPP_IMPLEMENT_EXCEPTION(MessageDigestIllegalArgumentException, MessageDigestException, 0)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(MessageDigestIllegalStateException, MessageDigestException, 1)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(MessageDigestExecutionException, MessageDigestException, 2)

} /* namespace messagedigest */
} /* namespace easyhttpcpp */
