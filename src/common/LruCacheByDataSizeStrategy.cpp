/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/LruCacheByDataSizeStrategy.h"

namespace easyhttpcpp {
namespace common {

static const std::string Tag = "LruCacheByDataSizeStrategy";

LruCacheByDataSizeStrategy::LruCacheByDataSizeStrategy(size_t maxSize) : m_pListener(NULL), m_maxSize(maxSize),
        m_totalSize(0)
{
}

LruCacheByDataSizeStrategy::~LruCacheByDataSizeStrategy()
{
    m_lruList.clear();
    m_lruListMap.clear();
}

void LruCacheByDataSizeStrategy::setListener(LruCacheByDataSizeStrategyListener* pListener)
{
    m_pListener = pListener;
}

bool LruCacheByDataSizeStrategy::add(const std::string& key, CacheInfoWithDataSize::Ptr pCacheInfo)
{
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "add: pCacheInfo is NULL.");
        return false;
    }
    bool ret = true;
    if (m_pListener != NULL) {
        ret = m_pListener->onAdd(key, pCacheInfo);
        if (!ret) {
            EASYHTTPCPP_LOG_D(Tag, "add: onAdd return false.");
        }
    }
    if (ret) {
        ret = addOrUpdate(key, pCacheInfo);
    }
    return ret;
}

bool LruCacheByDataSizeStrategy::update(const std::string& key, CacheInfoWithDataSize::Ptr pCacheInfo)
{
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "update: pCacheInfo is NULL.");
        return false;
    }
    bool ret = true;
    if (m_pListener != NULL) {
        ret = m_pListener->onUpdate(key, pCacheInfo);
        if (!ret) {
            EASYHTTPCPP_LOG_D(Tag, "update: onUpdate return false.");
        }
    }
    if (ret) {
        ret = addOrUpdate(key, pCacheInfo);
    }
    return ret;
}

bool LruCacheByDataSizeStrategy::remove(const std::string& key)
{
    bool ret = true;
    LruListMap::Iterator it = m_lruListMap.find(key);
    if (it != m_lruListMap.end()) {
        if (m_pListener != NULL) {
            ret = m_pListener->onRemove(key);
            if (!ret) {
                EASYHTTPCPP_LOG_D(Tag, "remove: onRemove return false.");
            }
        }
        if (ret) {
            CacheInfoWithDataSize::Ptr pValue = *(it->second);
            m_totalSize -= pValue->getDataSize();
            m_lruList.erase(it->second);
            m_lruListMap.erase(it);
        }
    } else {
        EASYHTTPCPP_LOG_D(Tag, "remove: not found key. [%s]", key.c_str());
        ret = false;
    }
    return ret;
}

CacheInfoWithDataSize::Ptr LruCacheByDataSizeStrategy::get(const std::string& key)
{
    LruListMap::Iterator it = m_lruListMap.find(key);
    if (it != m_lruListMap.end()) {
        CacheInfoWithDataSize::Ptr pCacheInfo = *(it->second);
        // move to latest position
        m_lruList.erase(it->second);
        m_lruList.push_front(pCacheInfo);

        it->second = m_lruList.begin();

        if (m_pListener != NULL) {
            if (!m_pListener->onGet(key, pCacheInfo)) {
                EASYHTTPCPP_LOG_D(Tag, "get: onGet return false.");
                return NULL;
            }
        }
        return newCacheInfo(pCacheInfo);
    } else {
        EASYHTTPCPP_LOG_D(Tag, "get: not found key. [%s]", key.c_str());
        return NULL;
    }
}

bool LruCacheByDataSizeStrategy::clear(bool mayDeleteIfBusy)
{
    LruKeyList keys;
    bool ret = createRemoveList(keys, mayDeleteIfBusy);
    for (LruKeyList::const_iterator it = keys.begin(); it != keys.end(); it++) {
        remove(*it);
    }
    return ret;
}

void LruCacheByDataSizeStrategy::reset()
{
    m_lruList.clear();
    m_lruListMap.clear();
    m_totalSize = 0;
}

bool LruCacheByDataSizeStrategy::makeSpace(size_t requestSize)
{
    return makeSpace(requestSize, "");
}

size_t LruCacheByDataSizeStrategy::getMaxSize() const
{
    return m_maxSize;
}

size_t LruCacheByDataSizeStrategy::getTotalSize() const
{
    return m_totalSize;
}

bool LruCacheByDataSizeStrategy::isEmpty() const
{
    return m_lruList.empty();
}

bool LruCacheByDataSizeStrategy::createRemoveList(LruKeyList& keys, bool mayDeleteIfBusy)
{
    for (LruList::const_iterator it = m_lruList.begin(); it != m_lruList.end(); it++) {
        CacheInfoWithDataSize::Ptr pCacheInfo = *it;
        keys.push_back(pCacheInfo->getKey());
    }
    return true;
}

bool LruCacheByDataSizeStrategy::createRemoveLruDataList(LruKeyList& keys, size_t removeSize)
{
    size_t targetSize = 0;
    LruList::reverse_iterator it = m_lruList.rbegin();
    while (targetSize < removeSize) {
        if (it == m_lruList.rend()) {
            EASYHTTPCPP_LOG_D(Tag, "createRemoveLruDataList: can not make free space. removeSize=%zu", removeSize);
            return false;
        }
        CacheInfoWithDataSize::Ptr pCacheInfo = *it;
        keys.push_front(pCacheInfo->getKey());
        targetSize += pCacheInfo->getDataSize();
        EASYHTTPCPP_LOG_D(Tag, "createRemoveLruDataList: dataDize=%zu key=%s", pCacheInfo->getDataSize(),
                pCacheInfo->getKey().c_str());

        ++it;
    }
    return true;
}

CacheInfoWithDataSize::Ptr LruCacheByDataSizeStrategy::newCacheInfo(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    return new CacheInfoWithDataSize(*pCacheInfo);
}

bool LruCacheByDataSizeStrategy::addOrUpdate(const std::string& key, CacheInfoWithDataSize::Ptr pCacheInfo)
{
    // check free space
    LruListMap::Iterator it = m_lruListMap.find(key);
    if (it != m_lruListMap.end()) {
        CacheInfoWithDataSize::Ptr pOldCacheInfo = *(it->second);
        if (pOldCacheInfo->getDataSize() < pCacheInfo->getDataSize()) {
            if (!makeSpace(pCacheInfo->getDataSize(), key)) {
                return false;
            }
        }
    } else {
        if (!makeSpace(pCacheInfo->getDataSize())) {
            return false;
        }
    }

    // find for update
    it = m_lruListMap.find(key);
    if (it != m_lruListMap.end()) {
        CacheInfoWithDataSize::Ptr pOldCacheInfo = *(it->second);
        m_totalSize -= pOldCacheInfo->getDataSize();
        // old value is removed from LruList
        m_lruList.erase(it->second);
    }
    // new value is pushed to top of LruList
    m_lruList.push_front(newCacheInfo(pCacheInfo));

    // update lruListMap
    m_lruListMap[key] = m_lruList.begin();

    m_totalSize += pCacheInfo->getDataSize();

    return true;
}

bool LruCacheByDataSizeStrategy::makeSpace(size_t requestSize, const std::string& updatedKey)
{
    if (m_totalSize + requestSize <= m_maxSize) {
        return true;
    }

    if (m_lruList.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "makeSpace: list is empty totalSize=%zu requestSize=%zu maxSize=%zu",
                m_totalSize, requestSize, m_maxSize);
        return false;
    }

    LruKeyList keys;
    if (createRemoveLruDataList(keys, m_totalSize + requestSize - m_maxSize)) {
        for (LruKeyList::const_iterator it = keys.begin(); it != keys.end(); it++) {
            // updatedKey do not remove because update later.
            if (updatedKey.empty() || updatedKey != *it) {
                remove(*it);
            }
        }
        return true;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "makeSpace: can not make space totalSize=%zu requestSize=%zu maxSize=%zu",
                m_totalSize, requestSize, m_maxSize);
        return false;
    }
}

} /* namespace common */
} /* namespace easyhttpcpp */
