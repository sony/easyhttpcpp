/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/HttpCache.h"

#include "HttpCacheInternal.h"

namespace easyhttpcpp {

HttpCache::~HttpCache()
{
}

HttpCache::Ptr HttpCache::createCache(const Poco::Path& path, size_t maxSize)
{
    return new HttpCacheInternal(path, maxSize);
}

} /* namespace easyhttpcpp */

