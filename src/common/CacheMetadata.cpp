/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CacheMetadata.h"

namespace easyhttpcpp {
namespace common {

CacheMetadata::CacheMetadata()
{
}

CacheMetadata::~CacheMetadata()
{
}

void CacheMetadata::setKey(const std::string& key)
{
    m_key = key;
}

const std::string& CacheMetadata::getKey() const
{
    return m_key;
}

} /* namespace common */
} /* namespace easyhttpcpp */
