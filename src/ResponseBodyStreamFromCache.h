/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODYSTREAMFROMCACHE_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODYSTREAMFROMCACHE_H_INCLUDED

#include "easyhttpcpp/HttpCache.h"
#include "easyhttpcpp/Response.h"

#include "ResponseBodyStreamInternal.h"

namespace easyhttpcpp {

class ResponseBodyStreamFromCache : public ResponseBodyStreamInternal {
public:
    ResponseBodyStreamFromCache(std::istream* pContent, Response::Ptr pResponse, HttpCache::Ptr pHttpCache);
    virtual ~ResponseBodyStreamFromCache();
    virtual void close();
private:
    std::istream* m_pContent;
    Response::Ptr m_pResponse;
    HttpCache::Ptr m_pHttpCache;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODYSTREAMFROMCACHE_H_INCLUDED */
