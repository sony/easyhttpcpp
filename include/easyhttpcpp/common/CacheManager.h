/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHEMANAGER_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHEMANAGER_H_INCLUDED

#include <istream>
#include <string>
#include <vector>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Mutex.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API CacheManager : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<CacheManager> Ptr;

    CacheManager(Cache::Ptr pL1Cache, Cache::Ptr pL2Cache);
    virtual ~CacheManager();
    virtual bool getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata);
    virtual bool getData(const std::string& key, std::istream*& pStream);
    virtual bool get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream);
    virtual bool putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata);
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path);
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<ByteArrayBuffer> pData);
    virtual bool remove(const std::string& key);
    virtual void releaseData(const std::string& key);
    virtual bool purge(bool mayDeleteIfBusy);

private:
    void bringToL1Cache(const std::string& key);
    // 0 : L1 cache (suppose memory cache)
    // 1 : L2 cache (suppose file cache)
    Cache::Ptr m_caches[2];
    Poco::FastMutex m_instanceMutex;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHEMANAGER_H_INCLUDED */
