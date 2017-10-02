/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/common/CoreLogger.h"

#include "HttpCacheInternal.h"
#include "HttpUtil.h"
#include "ResponseBodyStreamFromCache.h"

using easyhttpcpp::common::CacheManager;

namespace easyhttpcpp {

static const std::string Tag = "ResponseBodyStreamFromCache";

ResponseBodyStreamFromCache::ResponseBodyStreamFromCache(std::istream* pContent, Response::Ptr pResponse,
        HttpCache::Ptr pHttpCache) : ResponseBodyStreamInternal(*pContent), m_pContent(pContent),
        m_pResponse(pResponse), m_pHttpCache(pHttpCache)
{
}

ResponseBodyStreamFromCache::~ResponseBodyStreamFromCache()
{
    delete m_pContent;
}

void ResponseBodyStreamFromCache::close()
{
    Poco::Mutex::ScopedLock lock(m_instanceMutex);

    if (m_closed) {
        return;
    }

    ResponseBodyStreamInternal::close();

    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pHttpCache.get());
    Request::Ptr pRequest = m_pResponse->getRequest();
    std::string key = HttpUtil::makeCacheKey(pRequest);
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "close: can not make key.");
        return;
    }
    CacheManager::Ptr pCacheManager = pCacheInternal->getCacheManager();
    pCacheManager->releaseData(key);

    EASYHTTPCPP_LOG_D(Tag, "close: finished");
}

} /* namespace easyhttpcpp */
