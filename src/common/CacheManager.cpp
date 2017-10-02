/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/common/CoreLogger.h"

#define CACHE_MAX   2

namespace easyhttpcpp {
namespace common {

static const std::string Tag = "CacheManager";

CacheManager::CacheManager(Cache::Ptr pL1Cache, Cache::Ptr pL2Cache)
{
    m_caches[0] = pL1Cache;
    m_caches[1] = pL2Cache;
}

CacheManager::~CacheManager()
{
}

bool CacheManager::getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->getMetadata(key, pCacheMetadata)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] getMetadata : found.", i);
                if (i > 0) {
                    bringToL1Cache(key);
                }
                return true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] getMetadata : not found.", i);
            }
        }
    }
    return false;
}

bool CacheManager::getData(const std::string& key, std::istream*& pStream)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->getData(key, pStream)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] getData : found.", i);
                if (i > 0) {
                    bringToL1Cache(key);
                }
                return true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] getData : not found.", i);
            }
        }
    }
    return false;
}

bool CacheManager::get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pData)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->get(key, pCacheMetadata, pData)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] get : found.", i);
                if (i > 0) {
                    bringToL1Cache(key);
                }
                return true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] get : not found.", i);
            }
        }
    }
    return false;
}

bool CacheManager::putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    bool ret = false;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->putMetadata(key, pCacheMetadata)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] putMetadata : succeeded.", i);
                ret = true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] putMetadata : failed.", i);
            }
        }
    }
    return ret;
}

bool CacheManager::put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    bool ret = false;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->put(key, pCacheMetadata, path)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] put : succeeded.", i);
                ret = true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] put : failed.", i);
            }
        }
    }
    return ret;
}

bool CacheManager::put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
        Poco::SharedPtr<ByteArrayBuffer> pData)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    bool ret = false;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->put(key, pCacheMetadata, pData)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] put : succeeded.", i);
                ret = true;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] put : failed.", i);
            }
        }
    }
    return ret;
}

bool CacheManager::remove(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    int cacheCount = 0;
    int successCount = 0;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            cacheCount++;
            if (m_caches[i]->remove(key)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] remove : succeeded.", i);
                successCount++;
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] remove : failed.", i);
            }
        }
    }
    return (cacheCount > 0 && cacheCount == successCount);
}

void CacheManager::releaseData(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            m_caches[i]->releaseData(key);
            EASYHTTPCPP_LOG_D(Tag, "Cache[%d] releaseData", i);
        }
    }
}

bool CacheManager::purge(bool mayDeleteIfBusy)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    bool ret = true;
    for (int i = 0; i < CACHE_MAX; i++) {
        if (m_caches[i]) {
            if (m_caches[i]->purge(mayDeleteIfBusy)) {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] purge : succeeded.", i);
            } else {
                EASYHTTPCPP_LOG_D(Tag, "Cache[%d] purge : failed.", i);
                ret = false;
            }
        }
    }
    return ret;
}

void CacheManager::bringToL1Cache(const std::string& key)
{
    if (!m_caches[0]) {
        return;
    }

    // TODO: 
    // get from L2Cache
    // put to L1Cache
}

} /* namespace common */
} /* namespace easyhttpcpp */
