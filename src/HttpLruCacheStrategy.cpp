/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

#include "HttpCacheInfo.h"
#include "HttpLruCacheStrategy.h"

using easyhttpcpp::common::CacheInfoWithDataSize;
using easyhttpcpp::common::LruCacheByDataSizeStrategy;

namespace easyhttpcpp {

static const std::string Tag = "HttpLruCacheStrategy";

HttpLruCacheStrategy::HttpLruCacheStrategy(size_t maxSize) :
        LruCacheByDataSizeStrategy(maxSize)
{
}

HttpLruCacheStrategy::~HttpLruCacheStrategy()
{
}

bool HttpLruCacheStrategy::update(const std::string& key, CacheInfoWithDataSize::Ptr pCacheInfo)
{
    bool ret = LruCacheByDataSizeStrategy::update(key, pCacheInfo);
    if (ret) {
        // check reserved remove.
        HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
        if (pHttpCacheInfo->getDataRefCount() == 0 && pHttpCacheInfo->isReservedRemove()) {
            EASYHTTPCPP_LOG_D(Tag, "update: remove reserved cache. key=[%s]", key.c_str());
            ret = LruCacheByDataSizeStrategy::remove(key);
        }
    }
    return ret;
}

bool HttpLruCacheStrategy::remove(const std::string& key)
{
    LruListMap::Iterator it = m_lruListMap.find(key);
    if (it != m_lruListMap.end()) {
        // check dataRefCount.
        CacheInfoWithDataSize::Ptr pCacheInfo = *(it->second);
        HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
        if (pHttpCacheInfo->getDataRefCount() > 0) {
            if (!pHttpCacheInfo->isReservedRemove()) {
                // reserving remove.
                pHttpCacheInfo->setReservedRemove(true);
                EASYHTTPCPP_LOG_D(Tag, "remove: reserve remove. key=[%s]", key.c_str());
                LruCacheByDataSizeStrategy::update(key, pCacheInfo);
            }
            return true;
        }
        return LruCacheByDataSizeStrategy::remove(key);
    } else {
        return false;
    }
}

bool HttpLruCacheStrategy::clear(bool mayDeleteIfBusy)
{
    LruKeyList keys;
    bool ret = createRemoveList(keys, mayDeleteIfBusy);
    for (LruKeyList::const_iterator it = keys.begin(); it != keys.end(); it++) {
        if (!LruCacheByDataSizeStrategy::remove(*it)) {
            EASYHTTPCPP_LOG_D(Tag, "clear: Failed to remove. key=[%s]", it->c_str());
            ret = false;
        }
    }
    return ret;
}

bool HttpLruCacheStrategy::createRemoveList(LruKeyList& keys, bool mayDeleteIfBusy)
{
    bool ret = true;
    for (LruList::const_iterator it = m_lruList.begin(); it != m_lruList.end(); it++) {
        CacheInfoWithDataSize::Ptr pCacheInfo = *it;
        if (mayDeleteIfBusy) {
            keys.push_back(pCacheInfo->getKey());
        } else {
            HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
            if (pHttpCacheInfo->getDataRefCount() == 0) {
                keys.push_back(pCacheInfo->getKey());
            } else {
                ret = false;
            }
        }
    }
    return ret;
}

bool HttpLruCacheStrategy::createRemoveLruDataList(LruKeyList& keys, size_t removeSize)
{
    size_t targetSize = 0;
    LruList::reverse_iterator it = m_lruList.rbegin();
    while (targetSize < removeSize) {
        if (it == m_lruList.rend()) {
            return false;
        }
        CacheInfoWithDataSize::Ptr pCacheInfo = *it;
        HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
        if (pHttpCacheInfo->getDataRefCount() == 0) {
            keys.push_front(pCacheInfo->getKey());
            targetSize += pCacheInfo->getDataSize();
            EASYHTTPCPP_LOG_D(Tag, "createRemoveLruDataList: dataDize=%zu key=%s", pCacheInfo->getDataSize(),
                    pCacheInfo->getKey().c_str());
        }

        ++it;
    }
    return true;
}

CacheInfoWithDataSize::Ptr HttpLruCacheStrategy::newCacheInfo(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    CacheInfoWithDataSize::Ptr pNewCacheInfo =
            new HttpCacheInfo(*(static_cast<HttpCacheInfo*>(pCacheInfo.get())));
    return pNewCacheInfo;
}

} /* namespace easyhttpcpp */
