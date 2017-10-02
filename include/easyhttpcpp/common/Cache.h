/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHE_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHE_H_INCLUDED

#include <istream>
#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CacheMetadata.h"

namespace easyhttpcpp {
namespace common {

class Cache : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Cache> Ptr;

    virtual ~Cache()
    {
    }

    virtual bool getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata) = 0;
    virtual bool getData(const std::string& key, std::istream*& pStream) = 0;
    virtual bool get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream) = 0;
    virtual bool putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata) = 0;
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path) = 0;
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<ByteArrayBuffer> pData) = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual void releaseData(const std::string& key) = 0;
    virtual bool purge(bool mayDeleteIfBusy) = 0;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHE_H_INCLUDED */
