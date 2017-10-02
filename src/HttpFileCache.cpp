/*
 * Copyright 2017 Sony Corporation
 */

#include <fstream>

#include "Poco/Exception.h"
#include "Poco/File.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpCacheInfo.h"
#include "HttpCacheMetadata.h"
#include "HttpFileCache.h"
#include "HttpLruCacheStrategy.h"
#include "HttpUtil.h"

using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::CacheInfoWithDataSize;
using easyhttpcpp::common::CacheStrategy;

namespace easyhttpcpp {

static const std::string Tag = "HttpFileCache";

HttpFileCache::HttpFileCache(const Poco::Path& cacheRootDir, size_t maxSize) : m_cacheInitialized(false),
        m_cacheRootDir(cacheRootDir), m_maxSize(maxSize)
{
    Poco::Path databasePath = m_cacheRootDir;
    databasePath.append(Poco::Path(HttpConstants::Database::FileName));
    m_metadataDb = new HttpCacheDatabase(databasePath);
    m_lruCacheStrategy = new HttpLruCacheStrategy(m_maxSize);
    m_lruCacheStrategy->setListener(this);
}

HttpFileCache::~HttpFileCache()
{
}

bool HttpFileCache::getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    EASYHTTPCPP_LOG_D(Tag, "getMetadata key=%s", key.c_str());
    CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] not found in cache.", key.c_str());
        return false;
    }

    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
    if (pHttpCacheInfo->isReservedRemove()) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] is reserving delete.", key.c_str());
        return false;
    }

    HttpCacheMetadata* pHttpCacheMetadata = NULL;
    if (!m_metadataDb->getMetadata(key, pHttpCacheMetadata)) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] can not get from database.", key.c_str());
        return false;
    }
    CacheMetadata::Ptr pTempCacheMetadata = pHttpCacheMetadata;

    if (!m_metadataDb->updateLastAccessedSec(key)) {
        EASYHTTPCPP_LOG_D(Tag,
                "getMetadata : [%s] lastAccessedSec can not update. cache and database will; be inconsistent.",
                key.c_str());
        return false;
    }

    pCacheMetadata = pTempCacheMetadata;

    EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] succeeded.", key.c_str());
    return true;
}

bool HttpFileCache::getData(const std::string& key, std::istream*& pStream)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "getData : [%s] not found in cache.", key.c_str());
        return false;
    }

    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
    if (pHttpCacheInfo->isReservedRemove()) {
        EASYHTTPCPP_LOG_D(Tag, "getData : [%s] is reserving delete.", key.c_str());
        return false;
    }

    pStream = createStreamFromCache(key);
    if (pStream == NULL) {
        EASYHTTPCPP_LOG_D(Tag, "getData : [%s] can not create stream.", key.c_str());
        return false;
    }

    pHttpCacheInfo->addDataRef();
    m_lruCacheStrategy->update(key, pCacheInfo);

    EASYHTTPCPP_LOG_D(Tag, "getData : [%s] succeeded.", key.c_str());
    return true;
}

bool HttpFileCache::get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    // TODO: when m_reservedRemove is true, LRU list changed by LruCacheStrategy::get.
    // It is better to think of a good way.
    CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "get : [%s] not found in cache.", key.c_str());
        return false;
    }

    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
    if (pHttpCacheInfo->isReservedRemove()) {
        EASYHTTPCPP_LOG_D(Tag, "get : [%s] is reserving delete.", key.c_str());
        return false;
    }

    HttpCacheMetadata* pHttpCacheMetadata = NULL;
    if (!m_metadataDb->getMetadata(key, pHttpCacheMetadata)) {
        EASYHTTPCPP_LOG_D(Tag, "get : [%s] can not get from database.", key.c_str());
        return false;
    }
    CacheMetadata::Ptr pTempCacheMetadata = pHttpCacheMetadata;

    if (!m_metadataDb->updateLastAccessedSec(key)) {
        EASYHTTPCPP_LOG_D(Tag,
                "get : [%s] lastAccessedSec can not update. cache and database will be inconsistent.",
                key.c_str());
        return false;
    }

    pStream = createStreamFromCache(key);
    if (pStream == NULL) {
        EASYHTTPCPP_LOG_D(Tag, "getData : [%s] can not create stream.", key.c_str());
        return false;
    }

    pCacheMetadata = pTempCacheMetadata;
    pHttpCacheInfo->addDataRef();
    m_lruCacheStrategy->update(key, pCacheInfo);

    EASYHTTPCPP_LOG_D(Tag, "get : [%s] succeeded.", key.c_str());
    return true;
}

bool HttpFileCache::putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
    if (pCacheInfo) {
        HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
        if (pHttpCacheInfo->isReservedRemove()) {
            EASYHTTPCPP_LOG_D(Tag, "putMetadata : [%s] is reserving delete.", key.c_str());
            return false;
        }
        if (pHttpCacheInfo->getDataRefCount() != 0) {
            EASYHTTPCPP_LOG_D(Tag, "putMetadata : [%s] dataRefCont is not 0.", key.c_str());
            return false;
        }
    } else {
        EASYHTTPCPP_LOG_D(Tag, "putMetadata : not found in cache [%s]", key.c_str());
        return false;
    }

    if (!m_metadataDb->updateMetadata(key, static_cast<HttpCacheMetadata*> (pCacheMetadata.get()))) {
        EASYHTTPCPP_LOG_D(Tag, "putMetadata : [%s] update database failed.", key.c_str());
        return false;
    }

    return true;
}

bool HttpFileCache::put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    CacheInfoWithDataSize::Ptr pOldCacheInfo = m_lruCacheStrategy->get(key);
    if (pOldCacheInfo) {
        HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pOldCacheInfo.get());
        if (pHttpCacheInfo->isReservedRemove()) {
            EASYHTTPCPP_LOG_D(Tag, "put : [%s] is reserving delete.", key.c_str());
            return false;
        }
        if (pHttpCacheInfo->getDataRefCount() != 0) {
            EASYHTTPCPP_LOG_D(Tag, "put : [%s] dataRefCont is not 0.", key.c_str());
            return false;
        }

        // remove old cache
        removeInternal(key);
    }

    HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*> (pCacheMetadata.get());
    if (!m_lruCacheStrategy->makeSpace(pHttpCacheMetadata->getResponseBodySize())) {
        EASYHTTPCPP_LOG_D(Tag, "put : can not make cache space.");
        return false;
    }

    CacheInfoWithDataSize::Ptr pNewCacheInfo = new HttpCacheInfo(key, pHttpCacheMetadata->getResponseBodySize());

    Poco::File tempFile(path);
    std::string targetFilename = makeCachedFilename(key);
    try {
        tempFile.renameTo(targetFilename);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "can not move cache file. [%s] -> [%s] : [%s]", path.c_str(), targetFilename.c_str(),
                e.message().c_str());
        return false;
    }

    if (!m_metadataDb->updateMetadata(key, static_cast<HttpCacheMetadata*> (pCacheMetadata.get()))) {
        EASYHTTPCPP_LOG_D(Tag, "put : [%s] update database failed.", key.c_str());
        try {
            Poco::File cacheFile(targetFilename);
            cacheFile.remove(false);
        } catch (const Poco::Exception& e) {
            EASYHTTPCPP_LOG_D(Tag, "can not remove cache file. [%s] : [%s]", targetFilename.c_str(), e.message().c_str());
        }
        return false;
    }

    EASYHTTPCPP_LOG_D(Tag, "update key=%s", key.c_str());

    // update never fails, because already make free space.
    m_lruCacheStrategy->update(key, pNewCacheInfo);

    return true;
}

bool HttpFileCache::put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
        Poco::SharedPtr<ByteArrayBuffer> pData)
{
    EASYHTTPCPP_LOG_D(Tag, "put : [%s] ByteArrayBuffer is not supported.", key.c_str());
    return false;
}

bool HttpFileCache::remove(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    if (removeInternal(key)) {
        EASYHTTPCPP_LOG_D(Tag, "remove : [%s] succeeded.", key.c_str());
        return true;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "remove : [%s] succeeded.", key.c_str());
        return false;
    }
}

void HttpFileCache::releaseData(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
    if (!pCacheInfo) {
        EASYHTTPCPP_LOG_D(Tag, "releaseData : [%s] not found in cache.", key.c_str());
        return;
    }

    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
    if (pHttpCacheInfo->getDataRefCount() == 0) {
        EASYHTTPCPP_LOG_D(Tag, "releaseData : [%s] dataRefCont is already 0.", key.c_str());
    } else {
        pHttpCacheInfo->releaseDataRef();
        m_lruCacheStrategy->update(key, pCacheInfo);
    }

    EASYHTTPCPP_LOG_D(Tag, "releaseData : [%s] succeeded.", key.c_str());
}

bool HttpFileCache::purge(bool mayDeleteIfBusy)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }

    return m_lruCacheStrategy->clear(mayDeleteIfBusy);
}

std::istream* HttpFileCache::createStreamFromCache(const std::string& key)
{
    std::string filePath = makeCachedFilename(key);
    Poco::File file(filePath);
    try {
        if (!file.exists()) {
            EASYHTTPCPP_LOG_D(Tag, "createStreamFromCache: cached response body file is not found.[%s]", filePath.c_str());
            return NULL;
        }
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createStreamFromCache: Poco::Exception. [%s]", e.message().c_str());
        return NULL;
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createStreamFromCache: std::exception. [%s]", e.what());
        return NULL;
    }
    try {
        std::ifstream* pIfStream = new std::ifstream(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
        return pIfStream;
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createStreamFromCache: can not open cached response body [%s]", e.what());
        return NULL;
    }
}

size_t HttpFileCache::getSize()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (!initializeCache()) {
        return false;
    }
    return m_lruCacheStrategy->getTotalSize();
}

bool HttpFileCache::onAdd(const std::string& key, CacheInfoWithDataSize::Ptr value)
{
    return true;
}

bool HttpFileCache::onUpdate(const std::string& key, CacheInfoWithDataSize::Ptr value)
{
    return true;
}

bool HttpFileCache::onRemove(const std::string& key)
{
    if (!m_metadataDb->deleteMetadata(key)) {
        EASYHTTPCPP_LOG_D(Tag, "onRemove : Failed to delete from database. key=[%s]", key.c_str());
        return false;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "onRemove : delete from database. key=[%s]", key.c_str());
    }

    Poco::File file(makeCachedFilename(key));
    try {
        if (file.exists()) {
            file.remove();
        }
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Failed to remove cached file. [%s] : Details: %s", file.path().c_str(),
                e.message().c_str());
        return false;
    }

    return true;
}

bool HttpFileCache::onGet(const std::string& key, CacheInfoWithDataSize::Ptr value)
{
    return true;
}

bool HttpFileCache::onEnumerate(const HttpCacheEnumerationListener::EnumerationParam& param)
{
    if (m_lruCacheStrategy->makeSpace(param.m_responseBodySize)) {
        CacheInfoWithDataSize::Ptr pCacheInfo = new HttpCacheInfo(param.m_key, param.m_responseBodySize);
        m_lruCacheStrategy->add(param.m_key, pCacheInfo);
        return true;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "onEnumerate: cannot make cache space");
        return false;
    }
}

bool HttpFileCache::removeInternal(const std::string& key)
{
    return m_lruCacheStrategy->remove(key);
}

std::string HttpFileCache::makeCachedFilename(const std::string& key)
{
    return HttpUtil::makeCachedResponseBodyFilename(m_cacheRootDir, key);
}

bool HttpFileCache::initializeCache()
{
    if (m_cacheInitialized) {
        return true;
    }
    Poco::File file(m_cacheRootDir);
    try {
        file.createDirectories();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "create cache root directory failed %s", e.message().c_str());
        return false;
    }

    // initialize date base
    m_metadataDb->enumerate(this);

    m_cacheInitialized = true;

    return true;
}

} /* namespace easyhttpcpp */
