/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/File.h"
#include "Poco/NumberParser.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/CacheControl.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/Request.h"

#include "HttpCacheInternal.h"
#include "HttpCacheMetadata.h"
#include "HttpFileCache.h"
#include "HttpUtil.h"

using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheManager;
using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "HttpCacheInternal";

HttpCacheInternal::HttpCacheInternal(const Poco::Path& path, size_t maxSize) : m_maxSize(maxSize)
{
    m_cachePath = path;
    m_cacheRootDir = path;
    Poco::Path cacheDir(HttpConstants::Caches::CacheDir);
    m_cacheRootDir.append(cacheDir);
    m_pFileCache = new HttpFileCache(m_cacheRootDir, maxSize);
    m_pCacheManager = new CacheManager(NULL, m_pFileCache);
    Poco::Path tempDir(m_cacheRootDir);
    Poco::Path tempChildDir(HttpConstants::Caches::TempDir);
    tempDir.append(tempChildDir);
    m_cacheTempDir = tempDir.toString();
}

HttpCacheInternal::~HttpCacheInternal()
{
    m_pCacheManager = NULL;
    m_pFileCache = NULL;
}

const Poco::Path& HttpCacheInternal::getPath() const
{
    return m_cachePath;
}

void HttpCacheInternal::evictAll()
{
    if (!m_pCacheManager->purge(true)) {
        EASYHTTPCPP_LOG_D(Tag, "Failed to purge cache.");
        throw HttpExecutionException("Failed to purge cache.");
    }

    Poco::FastMutex::ScopedLock lock(m_directoryMutex);

    if (!FileUtil::removeDirsIfPresent(Poco::Path(m_cacheTempDir))) {
        EASYHTTPCPP_LOG_D(Tag, "Failed to remove cache temporary directory. [%s]", m_cacheTempDir.c_str());
        throw HttpExecutionException(StringUtil::format("Failed to remove cache temporary directory.[%s]",
                m_cacheTempDir.c_str()));
    }
}

size_t HttpCacheInternal::getMaxSize() const
{
    return m_maxSize;
}

size_t HttpCacheInternal::getSize()
{
    HttpFileCache* pFileCache = static_cast<HttpFileCache*> (m_pFileCache.get());
    return pFileCache->getSize();
}

const std::string& HttpCacheInternal::getTempDirectory()
{
    Poco::FastMutex::ScopedLock lock(m_directoryMutex);

    Poco::File file(m_cacheTempDir);
    if (!FileUtil::createDirsIfAbsent(file)) {
        EASYHTTPCPP_LOG_D(Tag, "create cache temp directory failed.");
        throw HttpExecutionException("can not access cache temporary directory.");
    }

    return m_cacheTempDir;
}

easyhttpcpp::common::CacheManager::Ptr HttpCacheInternal::getCacheManager() const
{
    return m_pCacheManager;
}

HttpCacheStrategy::Ptr HttpCacheInternal::createCacheStrategy(Request::Ptr pRequest)
{
    Poco::Timestamp now;
    unsigned long long nowAtEpoch = static_cast<unsigned long long>(now.epochTime());

    std::string key = HttpUtil::makeCacheKey(pRequest);
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "createCacheStrategy: can not make key.");
        return new HttpCacheStrategy(pRequest, NULL, nowAtEpoch);
    }

    // get CacheMetadata
    CacheMetadata::Ptr pCacheMetadata;
    if (!m_pCacheManager->getMetadata(key, pCacheMetadata)) {
        // not exist cache
        EASYHTTPCPP_LOG_D(Tag, "cache not found.");
        return new HttpCacheStrategy(pRequest, NULL, nowAtEpoch);
    }
    HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*> (pCacheMetadata.get());

    // create CacheResponse
    Response::Ptr pCacheResponse = createResponseFromCacheMetadata(pRequest, *pHttpCacheMetadata);

    // check CacheStrategy
    HttpCacheStrategy::Ptr pCacheStrategy = new HttpCacheStrategy(pRequest, pCacheResponse, nowAtEpoch);

    return pCacheStrategy;
}

void HttpCacheInternal::remove(Request::Ptr pRequest)
{
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, pRequest->getUrl());
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "remove: can not make key.");
        return;
    }
    m_pCacheManager->remove(key);
}

std::istream* HttpCacheInternal::createInputStreamFromCache(Request::Ptr pRequest)
{
    std::string key = HttpUtil::makeCacheKey(pRequest);
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "createInputStreamFromCache: can not make key.");
        return NULL;
    }
    std::istream* pStream = NULL;
    if (m_pCacheManager->getData(key, pStream)) {
        EASYHTTPCPP_LOG_D(Tag, "create response body stream from cache.");
        return pStream;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "can not create response body stream from cache.");
        return NULL;
    }
}

Response::Ptr HttpCacheInternal::createResponseFromCacheMetadata(Request::Ptr pRequest,
        HttpCacheMetadata& httpCacheMetadata)
{
    // create CacheControl from CacheMetadata
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(httpCacheMetadata.getResponseHeaders());

    // create Response
    Headers::Ptr pResponseHeaders = httpCacheMetadata.getResponseHeaders();
    Response::Builder builder;
    builder.setCode(httpCacheMetadata.getStatusCode()).setMessage(httpCacheMetadata.getStatusMessage()).
            setCacheControl(pCacheControl).setHeaders(pResponseHeaders).
            setRequest(pRequest).setSentRequestSec(httpCacheMetadata.getSentRequestAtEpoch()).
            setReceivedResponseSec(httpCacheMetadata.getReceivedResponseAtEpoch());

    if (pResponseHeaders->has(HttpConstants::HeaderNames::ContentLength)) {
        std::string contentLengthStr = pResponseHeaders->getValue(
                HttpConstants::HeaderNames::ContentLength, "-1");
        Poco::Int64 contentLength;
        if (Poco::NumberParser::tryParse64(contentLengthStr, contentLength)) {
            builder.setHasContentLength(true).setContentLength(static_cast<ssize_t> (contentLength));
        }
    }
    return builder.build();
}

} /* namespace easyhttpcpp */
