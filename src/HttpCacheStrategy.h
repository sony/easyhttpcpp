/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHESTRATEGY_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHESTRATEGY_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API HttpCacheStrategy : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpCacheStrategy> Ptr;

    HttpCacheStrategy(Request::Ptr pRequest, Response::Ptr pCacheResponse, unsigned long long nowAtEpoch);
    virtual ~HttpCacheStrategy();

    Request::Ptr getNetworkRequest();
    Response::Ptr getCachedResponse();

    static bool isAvailableToCache(Request::Ptr pRequest);
    static bool isValidCacheResponse(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse);
    static bool isCacheable(Response::Ptr pResponse);
    static Headers::Ptr combineCacheAndNetworkHeader(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse);
    static bool isInvalidCacheMethod(Response::Ptr pResponse);

private:
    void resolveCacheResponseAndNetworkRequest();
    unsigned long long calculateCacheResponseAge();
    unsigned long long calculateFreshnessLifetime();

    static bool isEndToEnd(const std::string& name);

    unsigned long long m_nowAtEpoch;
    Request::Ptr m_pRequest;
    Request::Ptr m_pNetworkRequest;
    Response::Ptr m_pCacheResponse;
    Poco::Timestamp m_responseDate;
    bool m_responseDateExisted;
    Poco::Timestamp m_lastModified;
    bool m_lastModifiedExisted;
    Poco::Timestamp m_expires;
    bool m_expiresExisted;
    long long m_ageSec;
    bool m_ageExisted;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHESTRATEGY_H_INCLUDED */
