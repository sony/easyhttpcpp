/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/ExceptionConstants.h"

namespace easyhttpcpp {
namespace common {

const std::string ExceptionConstants::ErrorCodePrefix = "EASYHTTPCPP-ERR-";

/**
 * Exception group codes
 */
const unsigned int ExceptionConstants::GroupCode::Core = 10;

/**
 * Exception subgroup codes
 */
const unsigned int ExceptionConstants::SubGroupCode::Common = 0;
const unsigned int ExceptionConstants::SubGroupCode::Db = 2;
const unsigned int ExceptionConstants::SubGroupCode::MessageDigest = 3;
const unsigned int ExceptionConstants::SubGroupCode::ExecutorService = 6;
const unsigned int ExceptionConstants::SubGroupCode::Http = 7;

} /* namespace common */
} /* namespace easyhttpcpp */
