/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPFILECACHE_H_INCLUDED
#define EASYHTTPCPP_HTTPFILECACHE_H_INCLUDED

#include "Poco/Path.h"

#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CacheStrategyListener.h"
#include "easyhttpcpp/common/LruCacheByDataSizeStrategy.h"

#include "HttpCacheDatabase.h"
#include "HttpCacheEnumerationListener.h"

namespace easyhttpcpp {

class HttpFileCache : public easyhttpcpp::common::Cache,
public easyhttpcpp::common::CacheStrategyListener<std::string,
        easyhttpcpp::common::CacheInfoWithDataSize::Ptr>, public HttpCacheEnumerationListener {
public:
    HttpFileCache(const Poco::Path& cacheRootDir, size_t maxSize);
    virtual ~HttpFileCache();
    virtual bool getMetadata(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr& pCacheMetadata);
    virtual bool getData(const std::string& key, std::istream*& pStream);
    virtual bool get(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr& pCacheMetadata,
            std::istream*& pStream);
    virtual bool putMetadata(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata);
    virtual bool put(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata,
            const std::string& path);
    virtual bool put(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<easyhttpcpp::common::ByteArrayBuffer> pData);
    virtual bool remove(const std::string& key);
    virtual void releaseData(const std::string& key);
    virtual bool purge(bool mayDeleteIfBusy);

    size_t getSize();

    virtual bool onAdd(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value);
    virtual bool onUpdate(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value);
    virtual bool onRemove(const std::string& key);
    virtual bool onGet(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value);

    virtual bool onEnumerate(const HttpCacheEnumerationListener::EnumerationParam& param);

private:
    std::istream* createStreamFromCache(const std::string& key);
    bool removeInternal(const std::string& key); 
    std::string makeCachedFilename(const std::string& key);
    void cleanupCache(const std::string& key);
    bool initializeCache();
    void deleteCacheFile();

    Poco::FastMutex m_instanceMutex;
    bool m_cacheInitialized;
    Poco::Path m_cacheRootDir;
    size_t m_maxSize;
    HttpCacheDatabase::Ptr m_metadataDb;
    easyhttpcpp::common::LruCacheByDataSizeStrategy::Ptr m_lruCacheStrategy;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPFILECACHE_H_INCLUDED */
