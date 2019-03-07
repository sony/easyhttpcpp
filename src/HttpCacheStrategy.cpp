/*
 * Copyright 2017 Sony Corporation
 */

#include <list>

#include "Poco/NumberParser.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpConstants.h"

#include "HttpCacheStrategy.h"
#include "HttpUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "HttpCacheStrategy";
static const long long OneDaySec = 24 * 60 * 60;

HttpCacheStrategy::HttpCacheStrategy(Request::Ptr pRequest, Response::Ptr pCacheResponse, unsigned long long nowAtEpoch)
        : m_nowAtEpoch(nowAtEpoch), m_pRequest(pRequest), m_pNetworkRequest(pRequest), m_pCacheResponse(pCacheResponse)
        , m_responseDateExisted(false), m_lastModifiedExisted(false), m_expiresExisted(false), m_ageSec(-1)
        , m_ageExisted(false)
{
    if (m_pCacheResponse) {
        // preparation frequent used stuff.
        if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::Date)) {
            if (HttpUtil::tryParseDate(m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::Date, ""),
                    m_responseDate)) {
                m_responseDateExisted = true;
            }
        }
        if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::LastModified)) {
            if (HttpUtil::tryParseDate(m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::LastModified, ""),
                    m_lastModified)) {
                m_lastModifiedExisted = true;
            }
        }
        if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::Expires)) {
            if (HttpUtil::tryParseDate(m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::Expires, ""),
                    m_expires)) {
                m_expiresExisted = true;
            }
        }
        if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::Age)) {
            m_ageExisted = true;
            Poco::Int64 value;
            if (Poco::NumberParser::tryParse64(m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::Age, ""),
                    value)) {
                m_ageSec = value;
            } else {
                m_ageSec = -1;
            }
        }
        resolveCacheResponseAndNetworkRequest();
    } else {
        // check onlyIfCached
        CacheControl::Ptr pCacheControl = m_pRequest->getCacheControl();
        if (pCacheControl && pCacheControl->isOnlyIfCached()) {
            EASYHTTPCPP_LOG_D(Tag, "can not send request when exist network request and only-if-cached");
            m_pNetworkRequest = NULL;
        }
    }
}

HttpCacheStrategy::~HttpCacheStrategy()
{
}

void HttpCacheStrategy::resolveCacheResponseAndNetworkRequest()
{
    m_pNetworkRequest = NULL;

    // Check CachControl
    unsigned long long ageSec = calculateCacheResponseAge();
    unsigned long long freshSec = calculateFreshnessLifetime();

    CacheControl::Ptr pRequestCacheControl = m_pRequest->getCacheControl();
    long long requestMaxAgeSec = -1;
    long long requestMinFreshSec = -1;
    long long requestMaxStaleSec = -1;
    bool isRequestOnlyIfCached = false;
    if (pRequestCacheControl) {
        requestMaxAgeSec = pRequestCacheControl->getMaxAgeSec();
        requestMinFreshSec = pRequestCacheControl->getMinFreshSec();
        requestMaxStaleSec = pRequestCacheControl->getMaxStaleSec();
        isRequestOnlyIfCached = pRequestCacheControl->isOnlyIfCached();
    }
    CacheControl::Ptr pResponseCacheControl = m_pCacheResponse->getCacheControl();
    bool isResponseMustRevalidate = false;
    bool isResponseNoCache = false;
    long long responseMaxAgeSec = -1;
    if (pResponseCacheControl) {
        isResponseMustRevalidate = pResponseCacheControl->isMustRevalidate();
        isResponseNoCache = pResponseCacheControl->isNoCache();
        responseMaxAgeSec = pResponseCacheControl->getMaxAgeSec();
    }

    if (requestMaxAgeSec != -1) {
        freshSec = std::min(freshSec, static_cast<unsigned long long>(requestMaxAgeSec));
    }
    unsigned long long minFreshSec = 0;
    if (requestMinFreshSec != -1) {
        minFreshSec = static_cast<unsigned long long>(requestMinFreshSec);
    }
    unsigned long long maxStaleSec = 0;
    if (!isResponseMustRevalidate && requestMaxStaleSec != -1) {
        maxStaleSec = static_cast<unsigned long long>(requestMaxStaleSec);
    }
    EASYHTTPCPP_LOG_D(Tag,
            "age condition: no-cache=%d ageSec=%llu minFreshSec=%llu freshSec=%llu maxStaleSec=%llu",
            isResponseNoCache, ageSec, minFreshSec, freshSec, maxStaleSec);
    if (!isResponseNoCache && ageSec + minFreshSec < freshSec + maxStaleSec) {
        std::list<std::pair<std::string, std::string> > extraHeaders;
        // RFC 2616 14.9.3 Modifications of the Basic Expiration Mechanism
        // if age is within range of stale, add warning header.
        if (ageSec + minFreshSec >= freshSec) {
            extraHeaders.push_back(std::make_pair(HttpConstants::HeaderNames::Warning,
                    HttpConstants::HeaderValues::ResponseIsStale));
            EASYHTTPCPP_LOG_D(Tag, "add warning header : %s", HttpConstants::HeaderValues::ResponseIsStale);
        }
        // RFC 2616 13.2.4 Expiration Calculations
        // if age is over one day and max-age exist and expires not exist, add warning header.
        if (ageSec > OneDaySec && responseMaxAgeSec == -1 && !m_expiresExisted) {
            extraHeaders.push_back(std::make_pair(HttpConstants::HeaderNames::Warning,
                    HttpConstants::HeaderValues::HeuristicExpiration));
            EASYHTTPCPP_LOG_D(Tag, "add warning header : %s", HttpConstants::HeaderValues::HeuristicExpiration);
        }
        if (extraHeaders.size() > 0) {
            Response::Builder responseBuilder(m_pCacheResponse);
            for (std::list<std::pair<std::string, std::string> >::const_iterator it = extraHeaders.begin();
                    it != extraHeaders.end(); it++) {
                responseBuilder.addHeader(it->first, it->second);
            }
            m_pCacheResponse = responseBuilder.build();
        }
        EASYHTTPCPP_LOG_D(Tag, "resolve to use cache response. ageSec + minFreshSec=%llu freshSec + maxStaleSec=%llu",
                ageSec + minFreshSec, freshSec + maxStaleSec);
        return;
    }

    // check onlyIfCached
    if (isRequestOnlyIfCached) {
        EASYHTTPCPP_LOG_D(Tag, "can not send request when exist network request and only-if-cached");
        m_pCacheResponse = NULL;
        return;
    }

    // check conditional request
    Headers conditionalRequestHeaders;
    if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::ETag)) {
        conditionalRequestHeaders.set(HttpConstants::HeaderNames::IfNoneMatch,
                m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::ETag, ""));
        EASYHTTPCPP_LOG_D(Tag, "conditional request: If-None-Match: ETag");
    } else if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::LastModified)) {
        conditionalRequestHeaders.set(HttpConstants::HeaderNames::IfModifiedSince, m_pCacheResponse->getHeaderValue(
                HttpConstants::HeaderNames::LastModified, ""));
        EASYHTTPCPP_LOG_D(Tag, "conditional request: If-Modified-Since: Last-Modified");
    } else if (m_pCacheResponse->hasHeader(HttpConstants::HeaderNames::Date)) {
        conditionalRequestHeaders.set(HttpConstants::HeaderNames::IfModifiedSince,
                m_pCacheResponse->getHeaderValue(HttpConstants::HeaderNames::Date, ""));
        EASYHTTPCPP_LOG_D(Tag, "conditional request: If-Modified-Since: Date");
    } else {
        EASYHTTPCPP_LOG_D(Tag, "not use cache response because could not make conditional request.");
        m_pCacheResponse = NULL;
    }

    // create network request
    Request::Builder requestBuilder(m_pRequest);
    for (Headers::HeaderMap::ConstIterator it = conditionalRequestHeaders.begin();
            it != conditionalRequestHeaders.end(); ++it) {
        requestBuilder.setHeader(it->first, it->second);
    }
    m_pNetworkRequest = requestBuilder.build();
}

Request::Ptr HttpCacheStrategy::getNetworkRequest()
{
    return m_pNetworkRequest;
}

Response::Ptr HttpCacheStrategy::getCachedResponse()
{
    return m_pCacheResponse;
}

unsigned long long HttpCacheStrategy::calculateCacheResponseAge()
{
    // reference RFC 2616 13.2.3 Age Calculations

    //             |<-- a: -->| a: apparentReceivedAge   
    // |<- responseDuration ->|<--- residentDuration  --->|
    // |-----------|----------|---------------------------|
    // sentRequest |          receiveResponse             now
    //             |
    //             Date header
    // receivedAge = max (apparentReceivedAge, Age header) : age from Date
    //
    // responseDuration : network delay time (from send to response)
    // receivedAge + responseDuration : Age of receive response (include network delay)
    // receivedAge + responseDuration + residentDuration : current Age (include network delay)

    std::time_t receivedResponseSec = m_pCacheResponse->getReceivedResponseSec();
    long long apparentReceivedAge = 0;
    if (m_responseDateExisted) {
        apparentReceivedAge = std::max(0LL, static_cast<long long> (receivedResponseSec - m_responseDate.epochTime()));
    }
    long long receivedAge = apparentReceivedAge;
    if (m_ageExisted) {
        receivedAge = std::max(apparentReceivedAge, m_ageSec);
    }
    long long responseDuration = receivedResponseSec - m_pCacheResponse->getSentRequestSec();
    long long residentDuration = m_nowAtEpoch - receivedResponseSec;

    return static_cast<unsigned long long>(receivedAge + responseDuration + residentDuration);
}

unsigned long long HttpCacheStrategy::calculateFreshnessLifetime()
{
    // RFC 2616 13.2.4 Expiration Calculations
    CacheControl::Ptr pResponseCacheControl = m_pCacheResponse->getCacheControl();
    if (pResponseCacheControl && pResponseCacheControl->getMaxAgeSec() != -1) {
        // MaxAge
        EASYHTTPCPP_LOG_D(Tag, "calculateFreshnessLifetime: use max-age");
        return static_cast<unsigned long long>(pResponseCacheControl->getMaxAgeSec());
    } else if (m_expiresExisted) {
        // Expires - (Date or receivedResponseSec))
        EASYHTTPCPP_LOG_D(Tag, "calculateFreshnessLifetime: use Expires");
        long long responseSec = m_pCacheResponse->getReceivedResponseSec();
        if (m_responseDateExisted) {
            responseSec = m_responseDate.epochTime();
        }
        long long delta = m_expires.epochTime() - responseSec;
        return static_cast<unsigned long long>(delta > 0 ? delta : 0);
    } else {
        // RFC 2616 13.2.4 Expiration Calculations
        // if response has Last-Modified, expiration value is 10% of interval.
        // RFC 2616 13.9 Side Effects of GET and HEAD
        // if with query url, cache must not treat response as fresh unless the server provides
        // an explicit expiration time.
        const std::string& url = m_pCacheResponse->getRequest()->getUrl();
        Poco::URI uri(url);
        if (m_lastModifiedExisted && uri.getQuery().empty()) {
            EASYHTTPCPP_LOG_D(Tag, "calculateFreshnessLifetime: use 10%% of Date and sentRequestSec");
            long long responseSec = m_pCacheResponse->getSentRequestSec();
            if (m_responseDateExisted) {
                responseSec = m_responseDate.epochTime();
            }
            long long delta = responseSec - m_lastModified.epochTime();
            return static_cast<unsigned long long>(delta > 0 ? (delta / 10) : 0);
        }
    }
    return 0;
}

bool HttpCacheStrategy::isAvailableToCache(Request::Ptr pRequest)
{
    if (pRequest->getMethod() != Request::HttpMethodGet) {
        return false;
    }
    if (pRequest->hasHeader(HttpConstants::HeaderNames::Authorization)) {
        return false;
    }
    CacheControl::Ptr pCacheControl = pRequest->getCacheControl();
    if (pCacheControl && pCacheControl->isNoCache()) {
        return false;
    }
    if (pRequest->hasHeader(HttpConstants::HeaderNames::IfModifiedSince)) {
        return false;
    }
    if (pRequest->hasHeader(HttpConstants::HeaderNames::IfNoneMatch)) {
        return false;
    }

    return true;
}

bool HttpCacheStrategy::isValidCacheResponse(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse)
{
    if (pNetworkResponse->getCode() == Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED) {
        EASYHTTPCPP_LOG_D(Tag, "isValidCacheResponse: valid cache. statusCode is Not Modified.");
        return true;
    }

    if (pCacheResponse->hasHeader(HttpConstants::HeaderNames::LastModified)) {
        std::string cacheLastModifiedStr = pCacheResponse->getHeaderValue(
                HttpConstants::HeaderNames::LastModified, "");
        Poco::Timestamp cacheLastModified;
        if (HttpUtil::tryParseDate(cacheLastModifiedStr, cacheLastModified)) {
            if (pNetworkResponse->hasHeader(HttpConstants::HeaderNames::LastModified)) {
                std::string networkLastModifiedStr = pNetworkResponse->getHeaderValue(
                        HttpConstants::HeaderNames::LastModified, "");
                Poco::Timestamp networkLastModified;
                if (HttpUtil::tryParseDate(networkLastModifiedStr, networkLastModified)) {
                    if (networkLastModified < cacheLastModified) {
                        EASYHTTPCPP_LOG_D(Tag,
                                "isValidCacheResponse: valid cache. cache Last-Modified is newer.");
                        return true;
                    }
                } else {
                    EASYHTTPCPP_LOG_D(Tag,
                            "isValidCacheResponse: Last-Modified in NetworkResponse is not supported format.[%s]",
                            cacheLastModifiedStr.c_str());
                }
            }
        } else {
            EASYHTTPCPP_LOG_D(Tag, "isValidCacheResponse: Last-Modified in CachedResponse is not supported format.[%s]",
                    cacheLastModifiedStr.c_str());
        }
    }
    return false;
}

bool HttpCacheStrategy::isCacheable(Response::Ptr pResponse)
{
    Request::Ptr pRequest = pResponse->getRequest();

    // check method
    if (pRequest->getMethod() != Request::HttpMethodGet) {
        EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached Method. [%s]",
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str());
        return false;
    }

    // check statusCode
    // this check was the OkHttp to reference.
    int statusCode = pResponse->getCode();
    switch (statusCode) {
            // RFC 7231 section 6.1
            // cacheable.
        case Poco::Net::HTTPResponse::HTTP_OK: // 200
        case Poco::Net::HTTPResponse::HTTP_NONAUTHORITATIVE: // 203
        case Poco::Net::HTTPResponse::HTTP_NO_CONTENT: // 204
        case Poco::Net::HTTPResponse::HTTP_MULTIPLE_CHOICES: // 300
        case Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY: // 301
        case Poco::Net::HTTPResponse::HTTP_NOT_FOUND: // 404
        case Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED: // 405
        case Poco::Net::HTTPResponse::HTTP_GONE: // 410
        case Poco::Net::HTTPResponse::HTTP_REQUESTURITOOLONG: // 414
        case Poco::Net::HTTPResponse::HTTP_NOT_IMPLEMENTED: // 501
            break;
        case 308: // 308:Permanent Redirect is not declaration in Poco
        case Poco::Net::HTTPResponse::HTTP_FOUND: // 302
        case Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT: // 307
        {
            // https://tools.ietf.org/html/rfc7234#section-3
            if (pResponse->hasHeader(HttpConstants::HeaderNames::Expires)) {
                break;
            }
            CacheControl::Ptr pResponseCacheControl = pResponse->getCacheControl();
            if (pResponseCacheControl && (pResponseCacheControl->getMaxAgeSec() != -1 ||
                    pResponseCacheControl->isPublic() ||
                    pResponseCacheControl->isPrivate())) {
                break;
            }
            EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached Status. [%d]", statusCode);
            return false;
        }
        default:
            EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached Status. [%d]", statusCode);
            return false;
    }

    // check request
    CacheControl::Ptr pRequestCacheControl = pRequest->getCacheControl();
    if (pRequestCacheControl && pRequestCacheControl->isNoStore()) {
        EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached. request Cache-Control contains no-store.");
        return false;
    }

    if (pRequest->hasHeader(HttpConstants::HeaderNames::Authorization)) {
        EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached. exist Authorization Header.");
        return false;
    }

    // check response
    CacheControl::Ptr pResponseCacheControl = pResponse->getCacheControl();
    if (pResponseCacheControl && pResponseCacheControl->isNoStore()) {
        EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached. exist response Cache-Control contains no-store.");
        return false;
    }
    if (!pResponse->hasContentLength() || pResponse->getContentLength() == -1) {
        if (Poco::icompare(pResponse->getHeaderValue(HttpConstants::HeaderNames::TransferEncoding, ""),
                HttpConstants::HeaderValues::Chunked) == 0) {
            return true;
        } else {
            EASYHTTPCPP_LOG_D(Tag, "isCacheable: Not Cached. not exist Content-Length.");
            return false;
        }
    }
    return true;
}

Headers::Ptr HttpCacheStrategy::combineCacheAndNetworkHeader(Response::Ptr pCacheResponse,
        Response::Ptr pNetworkResponse)
{
    // RFC 2616, 13.5.3 Combining Headers
    // remove 1xx warning.
    // use Hop-by-hop Headers of cache response
    // use cache response header,if not exist in network response.
    // use End-to-end Headers of network response
    // remove Content-Length
    Headers::Ptr pHeaders = new Headers();
    Headers::Ptr pCacheResponseHeaders = pCacheResponse->getHeaders();
    for (Headers::HeaderMap::ConstIterator it = pCacheResponseHeaders->begin(); it != pCacheResponseHeaders->end();
            it++) {
        if (Poco::icompare(it->first, HttpConstants::HeaderNames::Warning) == 0) {
            if (Poco::icompare(it->second, 1, "1") == 0) {
                // drop 1xx warning
                EASYHTTPCPP_LOG_D(Tag, "combineCacheAndNetworkHeader: drop from cached header . [%s]",
                        it->first.c_str(), it->second.c_str());
                continue;
            }
        }
        if (Poco::icompare(it->first, HttpConstants::HeaderNames::ContentLength) == 0) {
            pHeaders->add(it->first, it->second);
            continue;
        }
        if (!isEndToEnd(it->first) || !pNetworkResponse->hasHeader(it->first)) {
            pHeaders->add(it->first, it->second);
        }
    }

    Headers::Ptr pNetworkResponseHeaders = pNetworkResponse->getHeaders();
    for (Headers::HeaderMap::ConstIterator it = pNetworkResponseHeaders->begin(); it != pNetworkResponseHeaders->end();
            it++) {
        if (Poco::icompare(it->first, HttpConstants::HeaderNames::ContentLength) == 0) {
            // ignore Content-Length
            continue;
        }
        if (isEndToEnd(it->first)) {
            pHeaders->add(it->first, it->second);
        }
    }

    return pHeaders;
}

bool HttpCacheStrategy::isInvalidCacheMethod(Response::Ptr pResponse)
{
    if (!pResponse->isSuccessful()) {
        return false;
    }
    switch (pResponse->getRequest()->getMethod()) {
        case Request::HttpMethodPost:
        case Request::HttpMethodPatch:
        case Request::HttpMethodPut:
        case Request::HttpMethodDelete:
            EASYHTTPCPP_LOG_D(Tag, "cache invalidated method [%s]",
                    HttpUtil::httpMethodToString(pResponse->getRequest()->getMethod()).c_str());
            return true;
        default:
            return false;
    }
}

bool HttpCacheStrategy::isEndToEnd(const std::string& name)
{
    // RFC 2616 13.5.1 the following header is Hop-by-hop Headers (except the following is End-to-end Headers)
    return ((Poco::icompare(name, HttpConstants::HeaderNames::Connection) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::KeepAlive) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::ProxyAuthenticate) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::ProxyAuthorization) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::Te) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::Trailers) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::TransferEncoding) != 0) &&
            (Poco::icompare(name, HttpConstants::HeaderNames::Upgrade) != 0));
}

} /* namespace easyhttpcpp */

