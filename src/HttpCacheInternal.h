/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEINTERNAL_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEINTERNAL_H_INCLUDED

#include <istream>

#include "Poco/Mutex.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/HttpCache.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

#include "HttpCacheStrategy.h"

namespace easyhttpcpp {

class HttpCacheMetadata;

class EASYHTTPCPP_HTTP_INTERNAL_API HttpCacheInternal : public HttpCache {
public:
    HttpCacheInternal(const Poco::Path& path, size_t maxSize);
    virtual ~HttpCacheInternal();

    virtual const Poco::Path& getPath() const;
    virtual void evictAll();
    virtual size_t getMaxSize() const;
    virtual size_t getSize();

    HttpCacheStrategy::Ptr createCacheStrategy(Request::Ptr pRequest);
    void remove(Request::Ptr pRequest);
    std::istream* createInputStreamFromCache(Request::Ptr pRequest);
    const std::string& getTempDirectory();
    easyhttpcpp::common::CacheManager::Ptr getCacheManager() const;

private:
    Response::Ptr createResponseFromCacheMetadata(Request::Ptr pRequest, HttpCacheMetadata& httpCacheMetadata);

    size_t m_maxSize;
    easyhttpcpp::common::Cache::Ptr m_pFileCache;
    easyhttpcpp::common::CacheManager::Ptr m_pCacheManager;
    Poco::Path m_cachePath;
    Poco::Path m_cacheRootDir;
    std::string m_cacheTempDir;
    Poco::FastMutex m_directoryMutex;

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEINTERNAL_H_INCLUDED */
