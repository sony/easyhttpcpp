/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CacheInfoWithDataSize.h"

namespace easyhttpcpp {
namespace common {

CacheInfoWithDataSize::CacheInfoWithDataSize(const std::string& key, size_t dataSize) : m_key(key), m_dataSize(dataSize)
{
}

CacheInfoWithDataSize::CacheInfoWithDataSize(const CacheInfoWithDataSize& original)
{
    copyFrom(original);
}

CacheInfoWithDataSize::~CacheInfoWithDataSize()
{
}

CacheInfoWithDataSize& CacheInfoWithDataSize::CacheInfoWithDataSize::operator = (const CacheInfoWithDataSize& original)
{
	if (&original != this)
	{
        copyFrom(original);
	}
	return *this;
}

bool CacheInfoWithDataSize::operator == (const CacheInfoWithDataSize& target) const
{
    if (m_key == target.m_key && m_dataSize == target.m_dataSize) {
        return true;
    }
    return false;
}

void CacheInfoWithDataSize::setKey(const std::string& key)
{
    m_key = key;
}

const std::string& CacheInfoWithDataSize::getKey() const
{
    return m_key;
}

void CacheInfoWithDataSize::setDataSize(size_t dataSize)
{
    m_dataSize = dataSize;
}

size_t CacheInfoWithDataSize::getDataSize() const
{
    return m_dataSize;
}

void CacheInfoWithDataSize::copyFrom(const CacheInfoWithDataSize& original)
{
    m_key = original.m_key;
    m_dataSize = original.m_dataSize;
}

} /* namespace common */
} /* namespace easyhttpcpp */

