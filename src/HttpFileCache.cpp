/*
 * Copyright 2017 Sony Corporation
 */

#include <fstream>

#include "Poco/Exception.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpCacheInfo.h"
#include "HttpCacheMetadata.h"
#include "HttpFileCache.h"
#include "HttpInternalConstants.h"
#include "HttpLruCacheStrategy.h"
#include "HttpUtil.h"

using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::CacheInfoWithDataSize;
using easyhttpcpp::common::CacheStrategy;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::db::SqlDatabaseCorruptException;
using easyhttpcpp::db::SqlException;

namespace easyhttpcpp {

static const std::string Tag = "HttpFileCache";

HttpFileCache::HttpFileCache(const Poco::Path& cacheRootDir, size_t maxSize) : m_cacheInitialized(false),
        m_cacheRootDir(cacheRootDir.absolute()), m_maxSize(maxSize)
{
    Poco::Path databasePath = m_cacheRootDir;
    databasePath.append(Poco::Path(HttpInternalConstants::Database::FileName));
    m_pMetadataDb = new HttpCacheDatabase(new HttpCacheDatabaseOpenHelper(databasePath));
    m_lruCacheStrategy = new HttpLruCacheStrategy(m_maxSize);
    m_lruCacheStrategy->setListener(this);
}

HttpFileCache::~HttpFileCache()
{
    m_pMetadataDb->closeSqliteSession();
}

bool HttpFileCache::getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try {
        initializeCache();

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

        HttpCacheMetadata::Ptr pHttpCacheMetadata = m_pMetadataDb->getMetadata(key);
        if (!pHttpCacheMetadata) {
            EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] can not get from database.", key.c_str());
            return false;
        }

        if (!m_pMetadataDb->updateLastAccessedSec(key)) {
            EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] can not update last accessed sec. ignored.", key.c_str());
        }

        pCacheMetadata = pHttpCacheMetadata;
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : [%s] succeeded.", key.c_str());
        return true;
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }
}

bool HttpFileCache::getData(const std::string& key, std::istream*& pStream)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try {
        initializeCache();

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
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }
}

bool HttpFileCache::get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try{
        initializeCache();

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

        HttpCacheMetadata::Ptr pHttpCacheMetadata = m_pMetadataDb->getMetadata(key);
        if (!pHttpCacheMetadata) {
            EASYHTTPCPP_LOG_D(Tag, "get : [%s] can not get from database.", key.c_str());
            return false;
        }

        if (!m_pMetadataDb->updateLastAccessedSec(key)) {
            EASYHTTPCPP_LOG_D(Tag, "get : [%s] can not update last accessed sec. ignored.", key.c_str());
        }

        pStream = createStreamFromCache(key);
        if (pStream == NULL) {
            EASYHTTPCPP_LOG_D(Tag, "getData : [%s] can not create stream.", key.c_str());
            return false;
        }

        pCacheMetadata = pHttpCacheMetadata;
        pHttpCacheInfo->addDataRef();
        m_lruCacheStrategy->update(key, pCacheInfo);

        EASYHTTPCPP_LOG_D(Tag, "get : [%s] succeeded.", key.c_str());
        return true;
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "get : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }
}

bool HttpFileCache::putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try {
        initializeCache();

        CacheInfoWithDataSize::Ptr pCacheInfo = m_lruCacheStrategy->get(key);
        if (pCacheInfo) {
            HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pCacheInfo.get());
            if (pHttpCacheInfo->isReservedRemove()) {
                EASYHTTPCPP_LOG_D(Tag, "putMetadata : [%s] is reserving delete.", key.c_str());
                return false;
            }
            if (pHttpCacheInfo->getDataRefCount() != 0) {
                EASYHTTPCPP_LOG_D(Tag, "putMetadata : [%s] dataRefCount is not 0.", key.c_str());
                return false;
            }
        } else {
            EASYHTTPCPP_LOG_D(Tag, "putMetadata : not found in cache [%s]", key.c_str());
            return false;
        }

        m_pMetadataDb->updateMetadata(key, pCacheMetadata.unsafeCast<HttpCacheMetadata>());

        return true;
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "putMetadata : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }
}

bool HttpFileCache::put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    HttpCacheMetadata::Ptr pHttpCacheMetadata = pCacheMetadata.unsafeCast<HttpCacheMetadata>();
    try {
        initializeCache();

        CacheInfoWithDataSize::Ptr pOldCacheInfo = m_lruCacheStrategy->get(key);
        if (pOldCacheInfo) {
            HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*> (pOldCacheInfo.get());
            if (pHttpCacheInfo->isReservedRemove()) {
                EASYHTTPCPP_LOG_D(Tag, "put : [%s] is reserving delete.", key.c_str());
                return false;
            }
            if (pHttpCacheInfo->getDataRefCount() != 0) {
                EASYHTTPCPP_LOG_D(Tag, "put : [%s] dataRefCount is not 0.", key.c_str());
                return false;
            }

            // remove old cache
            removeInternal(key);
        }

        if (!m_lruCacheStrategy->makeSpace(pHttpCacheMetadata->getResponseBodySize())) {
            EASYHTTPCPP_LOG_D(Tag, "put : can not make cache space.");
            return false;
        }
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        // continue put processing.
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "put : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }

    CacheInfoWithDataSize::Ptr pNewCacheInfo = new HttpCacheInfo(key, pHttpCacheMetadata->getResponseBodySize());

    Poco::File tempFile(path);
    std::string targetFilename = makeCachedFilename(key);
    Poco::File cacheFile(targetFilename);
    if (!FileUtil::moveFile(tempFile, cacheFile)) {
        EASYHTTPCPP_LOG_D(Tag, "can not move cache file. [%s] -> [%s]", path.c_str(), targetFilename.c_str());
        return false;
    }

    try {
        initializeCache();

        m_pMetadataDb->updateMetadata(key, pHttpCacheMetadata);
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);

        if (!FileUtil::removeFileIfPresent(cacheFile)) {
            EASYHTTPCPP_LOG_D(Tag, "can not remove cache file. [%s]", targetFilename.c_str());
        }
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "put : [%s] update database failed.", key.c_str());
        if (!FileUtil::removeFileIfPresent(cacheFile)) {
            EASYHTTPCPP_LOG_D(Tag, "can not remove cache file. [%s]", targetFilename.c_str());
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

    try {
        initializeCache();

        if (removeInternal(key)) {
            EASYHTTPCPP_LOG_D(Tag, "remove : [%s] succeeded.", key.c_str());
            return true;
        } else {
            EASYHTTPCPP_LOG_D(Tag, "remove : [%s] succeeded.", key.c_str());
            return false;
        }
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        return false;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "remove : database error occurred. Details: %s", e.getMessage().c_str());
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
        EASYHTTPCPP_LOG_D(Tag, "releaseData : [%s] dataRefCount is already 0.", key.c_str());
    } else {
        pHttpCacheInfo->releaseDataRef();

        try {
            m_lruCacheStrategy->update(key, pCacheInfo);
        } catch (const SqlDatabaseCorruptException& e) {
            deleteCorruptedCacheFile(__func__, e);
        } catch (const SqlException& e) {
            EASYHTTPCPP_LOG_D(Tag, "remove : database error occurred. Details: %s", e.getMessage().c_str());
        }
    }

    EASYHTTPCPP_LOG_D(Tag, "releaseData : [%s] succeeded.", key.c_str());
}

bool HttpFileCache::purge(bool mayDeleteIfBusy)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try {
        initializeCache();

        bool ret = m_lruCacheStrategy->clear(mayDeleteIfBusy);

        // delete database file if HttpLruCacheStrategy::clear returns true
        // because if false returned, cache file is now reading.
        if (ret) {
            deleteCacheFile();
        }
        return ret;
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        // purge is successful.
        return true;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "purge : database error occurred. Details: %s", e.getMessage().c_str());
        return false;
    }
}

void HttpFileCache::deleteCacheFile()
{
    // session should be closed before remove the data.
    m_pMetadataDb->closeSqliteSession();
    m_pMetadataDb->deleteDatabaseFile();
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
        Poco::FileInputStream* pIfStream =
                new Poco::FileInputStream(filePath, std::ios_base::in | std::ios_base::binary);
        return pIfStream;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createStreamFromCache: can not open cached response body [%s]", e.message().c_str());
        return NULL;
    }
}

size_t HttpFileCache::getSize()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    try {
        initializeCache();
    } catch (const SqlDatabaseCorruptException& e) {
        deleteCorruptedCacheFile(__func__, e);
        throw HttpExecutionException("Cache database is corrupted. Check getCause() for details.", e);
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getSize : database error occurred. Details: %s", e.getMessage().c_str());
        throw HttpExecutionException("Cache database error occured. Check getCause() for details.", e);
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
    try {
        if (!m_pMetadataDb->deleteMetadata(key)) {
            EASYHTTPCPP_LOG_D(Tag, "onRemove : Failed to delete from database. key=[%s]", key.c_str());
            return false;
        } else {
            EASYHTTPCPP_LOG_D(Tag, "onRemove : delete from database. key=[%s]", key.c_str());
        }
    } catch (const SqlDatabaseCorruptException& e) {
        EASYHTTPCPP_LOG_D(Tag, "onRemove : cache database is corrupted. Details: %s", e.getMessage().c_str());
        throw;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "onRemove : deleteMetadata failed. Details: %s", e.getMessage().c_str());
        return false;
    }

    Poco::File file(makeCachedFilename(key));
    if (!FileUtil::removeFileIfPresent(file)) {
        EASYHTTPCPP_LOG_D(Tag, "Failed to remove cached file. [%s]", file.path().c_str());
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

void HttpFileCache::initializeCache()
{
    if (m_cacheInitialized) {
        return;
    }
    Poco::File file(m_cacheRootDir);
    if (!FileUtil::createDirsIfAbsent(file)) {
        EASYHTTPCPP_LOG_D(Tag, "Create cache root directory failed.");
        return;
    }

    // initialize date base
    m_pMetadataDb->enumerate(this);

    m_cacheInitialized = true;
}

void HttpFileCache::deleteCorruptedCacheFile(const std::string& funcName, const easyhttpcpp::db::SqlException& e)
{
    EASYHTTPCPP_LOG_W(Tag, "%s: cache database got corrupted.", funcName.c_str());
    EASYHTTPCPP_LOG_D(Tag, "%s: cache database got corrupted. Details: %s", funcName.c_str(),
            e.getMessage().c_str());

    m_cacheInitialized = false;

    // reset LRU cache
    m_lruCacheStrategy->reset();

    // delete cache database
    deleteCacheFile();

    // delete cache response body
    Poco::File cacheDir(m_cacheRootDir);
    std::vector<Poco::File> fileLists;
    cacheDir.list(fileLists);
    for (size_t i = 0; i < fileLists.size(); i++) {
        FileUtil::removeFileIfPresent(fileLists[i]);
    }

    EASYHTTPCPP_LOG_W(Tag, "%s: Deleted the cache database and cached files because the Cache database was corrupted.",
            funcName.c_str());
}

} /* namespace easyhttpcpp */
