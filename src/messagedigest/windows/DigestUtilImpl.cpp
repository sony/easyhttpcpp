/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/messagedigest/DigestUtil.h"

#include "DigestUtilImpl.h"

namespace easyhttpcpp {
namespace messagedigest {

std::string DigestUtilImpl::createHashedFileName(const std::string& key)
{
    return DigestUtil::sha1Hex(key);
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */
