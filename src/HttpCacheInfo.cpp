/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

#include "HttpCacheInfo.h"

namespace easyhttpcpp {

static const std::string Tag = "HttpCacheInfo";

HttpCacheInfo::HttpCacheInfo(const std::string& key, size_t dataSize) :
        CacheInfoWithDataSize(key, dataSize), m_reservedRemove(false), m_dataRefCount(0)
{
}

HttpCacheInfo::HttpCacheInfo(const HttpCacheInfo& original) : CacheInfoWithDataSize(original)
{
    m_reservedRemove = original.m_reservedRemove;
    m_dataRefCount = original.m_dataRefCount;
}

HttpCacheInfo::~HttpCacheInfo()
{
}

HttpCacheInfo& HttpCacheInfo::HttpCacheInfo::operator = (const HttpCacheInfo& original)
{
    if (&original != this) {
        CacheInfoWithDataSize::copyFrom(original);
        m_reservedRemove = original.m_reservedRemove;
        m_dataRefCount = original.m_dataRefCount;
    }
    return *this;
}

bool HttpCacheInfo::operator == (const HttpCacheInfo& target) const
{
    if (getKey() == target.getKey() && getDataSize() == target.getDataSize() &&
        m_reservedRemove == target.m_reservedRemove && m_dataRefCount == target.m_dataRefCount) {
        return true;
    }
    return false;
}

void HttpCacheInfo::setReservedRemove(bool reservedRemove)
{
    m_reservedRemove = reservedRemove;
}

bool HttpCacheInfo::isReservedRemove() const
{
    return m_reservedRemove;
}

void HttpCacheInfo::addDataRef()
{
    m_dataRefCount++;
}

void HttpCacheInfo::releaseDataRef()
{
    if (m_dataRefCount == 0) {
        EASYHTTPCPP_LOG_D(Tag, "releaseDataRefCount: dataRefCount is already 0.");
        return;
    }
    m_dataRefCount--;
}
unsigned int HttpCacheInfo::getDataRefCount() const
{
    return m_dataRefCount;
}

} /* namespace easyhttpcpp */


