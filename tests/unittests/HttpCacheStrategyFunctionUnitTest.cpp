/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/NumberFormatter.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/RequestBody.h"

#include "easyhttpcpp/Request.h"

#include "HttpCacheStrategy.h"
#include "HttpUtil.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const char* const DefaultRequestBody = "test request body";
static const char* const DefaultContentType = "text/plain";

static const char* const HeaderAuthorization = "Authorization";
static const char* const AuthorizationValue = "authorization value";

static const char* const HeaderIfModifiedSince = "If-Modified-Since";
static const char* const IfModifiedSinceValue = "Fri, 05 Aug 2016 12:00:00 GMT";

static const char* const HeaderIfNoneMatch = "If-None-Match";
static const char* const IfNoneMatchValue = "tag-00001";

static const char* const HeaderLastModified = "Last-Modified";
static const char* const LastModifiedValue = "Fri, 05 Aug 2016 12:00:00 GMT";
static const char* const LastModifiedSmall = "Fri, 05 Aug 2016 12:00:00 GMT";
static const char* const LastModifiedLarge = "Fri, 05 Aug 2016 12:00:01 GMT";
static const char* const LastModifiedInvalid = "Fri, 05 XXX 2016 12:00:01 GMT";

static const char* const HeaderExpires = "Expires";
static const char* const ExpiresValue = "Fri, 05 Aug 2016 12:00:00 GMT";

static const char* const HeaderTransferEncoding = "Transfer-Encoding";

static const char* const HeaderWarning = "Warning";
static const char* const Warning110Value = "110 - Response is stale.";
static const char* const Warning113Value = "113 - Heuristic expiration.";
static const char* const Warning2xxValue = "299 - Misc warning.";

static const char* const HeaderDate = "Date";
static const char* const DateValue1 = "Fri, 05 Aug 2016 10:00:00 GMT";
static const char* const DateValue2 = "Fri, 05 Aug 2016 12:00:00 GMT";

static const char* const HeaderConnection = "Connection";
static const char* const ConnectionValue1 = "Close";
static const char* const ConnectionValue2 = "Upgrade";

static const long long MaxAgeValue = 3600;

class HttpCacheStrategyFunctionUnitTest : public testing::Test {
};

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenPostMethod)
{
    // Given: POST Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenHeadMethod)
{
    // Given: HEAD Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpHead().build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenPutMethod)
{
    // Given: PUT Method
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPut(pRequestBody).build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenDeleteMethod)
{
    // Given: DELETE Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpDelete().build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenGetMethodAndAuthorizationHeader)
{
    // Given: GET Method, Authorization Header
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setHeader(HeaderAuthorization, AuthorizationValue).build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenGetMethodAndNoCache)
{
    // Given: GET Method, no-cache
    CacheControl::Builder cacheControlBuilder;
    cacheControlBuilder.setNoCache(true);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setCacheControl(cacheControlBuilder.build()).build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenGetMethodAndIfModifiedSince)
{
    // Given: GET Method, If-Modified-Since
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setHeader(HeaderIfModifiedSince, IfModifiedSinceValue).build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsFalse_WhenGetMethodAndIfNoneMatch)
{
    // Given: GET Method, If-None-Match
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setHeader(HeaderIfNoneMatch, IfNoneMatchValue).build();

    // When: call isAvailableToCache
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isAvailableToCache_ReturnsTrue_WhenGetMethod)
{
    // Given: GET Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.build();

    // When: call isAvailableToCache
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isAvailableToCache(pRequest));
}

TEST(HttpCacheStrategyFunctionUnitTest, isValidCacheResponse_ReturnsTrue_WhenStatusCodeIs304)
{
    // Given: statusCode = NotModified
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED).build();

    // When: call isValidCacheResponse
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isValidCacheResponse(NULL, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isValidCacheResponse_ReturnsFalse_WhenLastModifiedNotExistsInCacheResponse)
{
    // Given: not exist Last-Modified in cached Response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsFalse_WhenLastModifiedExistsInCacheResponseAndNotExistsInNetworkResponse)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedValue).build();

    // not exist LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsFalse_WhenLastModifiedInCacheResponseIsLessThanNetworkResponse)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedSmall).build();

    // LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setHeader(HeaderLastModified, LastModifiedLarge).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsFalse_WhenLastModifiedInCacheResponseEqualsNetworkResponse)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedSmall).build();

    // LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setHeader(HeaderLastModified, LastModifiedSmall).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsTrue_WhenLastModifiedInCacheResponseIsGreaterThanNetworkResponse)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedLarge).build();

    // LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setHeader(HeaderLastModified, LastModifiedSmall).build();

    // When: call isValidCacheResponse
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsFalse_WhenLastModifiedInCacheResponseIsInvalidFormat)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedInvalid).
            build();

    // LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setHeader(HeaderLastModified, LastModifiedSmall).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest,
        isValidCacheResponse_ReturnsFalse_WhenLastModifiedInNetworkResponseIsInvalidFormat)
{
    // Given: LastModified in cached response
    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setHeader(HeaderLastModified, LastModifiedLarge).
            build();

    // LastModified in network response
    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setHeader(HeaderLastModified, LastModifiedInvalid).build();

    // When: call isValidCacheResponse
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isValidCacheResponse(pCachedResponse, pNetworkResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isCacheable_ReturnsFalse_WhenPostMethod)
{
    // Given: POST Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();
    
    // When:: call isCacheable
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isCacheable(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isCacheable_ReturnsFalse_WhenPutMethod)
{
    // Given: PUT Method
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPut(pRequestBody).build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();
    
    // When:: call isCacheable
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isCacheable(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isCacheable_ReturnsFalse_WhenDeleteMethod)
{
    // Given: DELETE Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpDelete().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();
    
    // When:: call isCacheable
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isCacheable(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isCacheable_ReturnsFalse_WhenPatchMethod)
{
    // Given: PATCH Method
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPatch(pRequestBody).build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();
    
    // When:: call isCacheable
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isCacheable(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isCacheable_ReturnsFalse_WhenHeadMethod)
{
    // Given: HEAD Method
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpHead().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();
    
    // When:: call isCacheable
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isCacheable(pResponse));
}

class IsCacheableParam {
public:
    int statusCode;
    bool hasContentLength;
    ssize_t contentLength;
    bool hasExpires;
    bool hasResponseMaxAge;
    bool hasResponsePublic;
    bool hasResponsePrivate;
    bool hasResponseNoStore;
    bool hasRequestNoCache;
    bool hasAuthorization;
    bool hasTransferEncoding;
    const char* pTransferEncodingValue;
    bool retValue;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "statusCode             : " + Poco::NumberFormatter::format(statusCode) + "\n" +
                "hasContentLength       : " + StringUtil::boolToString(hasContentLength) + "\n" +
                "contentLength          : " + Poco::NumberFormatter::format(contentLength) + "\n" +
                "hasExpires             : " + StringUtil::boolToString(hasExpires) + "\n" +
                "hasResponseMaxAge      : " + StringUtil::boolToString(hasResponseMaxAge) + "\n" +
                "hasResponsePublic      : " + StringUtil::boolToString(hasResponsePublic) + "\n" +
                "hasResponsePrivate     : " + StringUtil::boolToString(hasResponsePrivate) + "\n" +
                "hasResponseNoStore     : " + StringUtil::boolToString(hasResponseNoStore) + "\n" +
                "hasRequestNoCache      : " + StringUtil::boolToString(hasRequestNoCache) + "\n" +
                "hasAuthorization       : " + StringUtil::boolToString(hasAuthorization) + "\n" +
                "hasTransferEncoding    : " + StringUtil::boolToString(hasTransferEncoding) + "\n" +
                "pTransferEncodingValue : " + (pTransferEncodingValue == NULL ? "" : pTransferEncodingValue) + "\n" +
                "retValue               : " + StringUtil::boolToString(retValue) + "\n";
        return ret;
    }
};
static const IsCacheableParam IsCacheableData[] = {
    {   // (0) GET Method, statusCode==200, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (1) GET Method, statusCode==203, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_NONAUTHORITATIVE,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (2) GET Method, statusCode==204, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_NO_CONTENT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (3) GET Method, statusCode==300, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_MULTIPLE_CHOICES,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (4) GET Method, statusCode==301, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (5) GET Method, statusCode==404, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_NOT_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (6) GET Method, statusCode==405, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (7) GET Method, statusCode==410, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_GONE,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (8) GET Method, statusCode==414, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_REQUESTURITOOLONG,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (9) GET Method, statusCode==501, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_NOT_IMPLEMENTED,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (10) GET Method, statusCode==302, Content-Length あり, Expires あり
        Poco::Net::HTTPResponse::HTTP_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        true,  // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (11) GET Method, statusCode==302, Content-Length あり, Response の Cache-Control に max-age あり
        Poco::Net::HTTPResponse::HTTP_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        true,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (12) GET Method, statusCode==302, Content-Length あり, Response の Cache-Control に  public あり
        Poco::Net::HTTPResponse::HTTP_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        true,  // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (13) GET Method, statusCode==302, Content-Length あり, Response の Cache-Control に  private あり
        Poco::Net::HTTPResponse::HTTP_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        true,  // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (14) GET Method, statusCode==302, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_FOUND,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (15) GET Method, statusCode==307, Content-Length あり, Expires あり
        Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        true,  // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (16) GET Method, statusCode==307, Content-Length あり, Response の Cache-Control に max-age あり
        Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        true,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (17) GET Method, statusCode==307, Content-Length あり, Response の Cache-Control に  public あり
        Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        true,  // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (18) GET Method, statusCode==307, Content-Length あり, Response の Cache-Control に  private あり
        Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        true,  // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (19) GET Method, statusCode==307, Content-Length あり
        Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (20) GET Method, statusCode==200, Content-Length あり, Request の Cache-Control に no-cache あり
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        true,  // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (21) GET Method, statusCode==200, Content-Length あり, Authorization あり
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false,  // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        true,  // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (22) GET Method, statusCode==200, Content-Length あり, Response のCache-Control に no-store あり
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        true, // hasContentLength;
        50,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        true,  // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (23) GET Method, statusCode==200, Content-Length なし, Transfer-Encoding == chunked
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        false, // hasContentLength;
        -1,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        true,  // hasTransferEncoding;
        "chunked",  // pTransferEncodingValue;
        true   // retValue;
    },
    {   // (24) GET Method, statusCode==200, Content-Length なし, Transfer-Encoding なし
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        false, // hasContentLength;
        -1,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (25) GET Method, statusCode==200, Content-Length なし, Transfer-Encoding が空
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        false, // hasContentLength;
        -1,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasRequestNoCache;
        false, // hasAuthorization;
        true,  // hasTransferEncoding;
        "",    // pTransferEncodingValue;
        false  // retValue;
    },
    {   // (26) GET Method, statusCode==200, Content-Length なし, Transfer-Encoding が空
        Poco::Net::HTTPResponse::HTTP_OK,  // statusCode;
        true,  // hasContentLength;
        -1,    // contentLength;
        false, // hasExpires;
        false, // hasResponseMaxAge;
        false, // hasResponsePublic;
        false, // hasResponsePrivate;
        false, // hasResponseNoStore;
        false, // hasResponseNoStore;
        false, // hasAuthorization;
        false, // hasTransferEncoding;
        NULL,  // pTransferEncodingValue;
        false  // retValue;
    },
};
class IsCacheableTest : public HttpCacheStrategyFunctionUnitTest ,
public testing::WithParamInterface<IsCacheableParam> {
};
INSTANTIATE_TEST_CASE_P(HttpCacheStrategyFunctionUnitTest, IsCacheableTest,
        testing::ValuesIn(IsCacheableData));

TEST_P(IsCacheableTest, isCacheable_ReturnsIsCacheableResult_WhenCombinationByStatusCodeAndRequestAndResponseHeader)
{
    IsCacheableParam& param = (IsCacheableParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given:
    Request::Builder requestBuilder;
    if (param.hasRequestNoCache) {
        CacheControl::Builder requestCacheControlBuilder;
        requestBuilder.setCacheControl(requestCacheControlBuilder.setNoCache(true).build());
    }
    if (param.hasAuthorization) {
        requestBuilder.setHeader(HeaderAuthorization, AuthorizationValue);
    }
    Request::Ptr pRequest = requestBuilder.build();

    Response::Builder responseBuilder;
    if (param.hasContentLength) {
        responseBuilder.setHasContentLength(true);
        responseBuilder.setContentLength(param.contentLength);
    }
    if (param.hasExpires) {
        responseBuilder.setHeader(HeaderExpires, ExpiresValue);
    }
    if (param.hasResponseMaxAge || param.hasResponsePublic || param.hasResponsePrivate || param.hasResponseNoStore) {
        CacheControl::Builder responseCacheControlBuilder;
        if (param.hasResponseMaxAge) {
            responseCacheControlBuilder.setMaxAgeSec(MaxAgeValue);
        }
        if (param.hasResponsePublic) {
            responseCacheControlBuilder.setPublic(true);
        }
        if (param.hasResponsePrivate) {
            responseCacheControlBuilder.setPrivate(true);
        }
        if (param.hasResponseNoStore) {
            responseCacheControlBuilder.setNoStore(true);
        }
        responseBuilder.setCacheControl(responseCacheControlBuilder.build());
    }
    if (param.hasTransferEncoding) {
        responseBuilder.setHeader(HeaderTransferEncoding, param.pTransferEncodingValue);
    }

    Response::Ptr pResponse = responseBuilder.setCode(param.statusCode).setRequest(pRequest).build();

    // When: call isCacheable
    // Then: check return value
    EXPECT_EQ(param.retValue, HttpCacheStrategy::isCacheable(pResponse));
}

class IsInvalidCacheMethodParam {
public:
    int statusCode;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "statusCode : " + Poco::NumberFormatter::format(statusCode) + "\n";
        return ret;
    }
};
static const IsInvalidCacheMethodParam IsInvalidCacheMethodData[] = {
    {   // statucCode==300
        Poco::Net::HTTPResponse::HTTP_MULTIPLE_CHOICES // statusCode
    },
    {   // statucCode==400
        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST // statusCode
    },
    {   // statucCode==500
        Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR // statusCode
    }
};
class IsInvalidCacheMethodTest : public HttpCacheStrategyFunctionUnitTest ,
public testing::WithParamInterface<IsInvalidCacheMethodParam> {
};
INSTANTIATE_TEST_CASE_P(HttpCacheStrategyFunctionUnitTest, IsInvalidCacheMethodTest,
        testing::ValuesIn(IsInvalidCacheMethodData));

TEST_P(IsInvalidCacheMethodTest, isInvalidCacheMethod_RetunsFalse_WhenNotSuccessfulStatusCode)
{
    IsInvalidCacheMethodParam& param = (IsInvalidCacheMethodParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given: set method and statusCode
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(param.statusCode).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return check result
    EXPECT_FALSE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsTrue_WhenPostMethod)
{
    // Given: set POST method and statusCode = HTTP_OK
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsTrue_WhenPatchMethod)
{
    // Given: set PATCH method and statusCode = HTTP_OK
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPatch(pRequestBody).build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsTrue_WhenPutMethod)
{
    // Given: set PUT method and statusCode = HTTP_OK
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPut(pRequestBody).build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsTrue_WhenDeleteMethod)
{
    // Given: set DELETE method and statusCode = HTTP_OK
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpDelete().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return true
    EXPECT_TRUE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsFalse_WhenGetMethod)
{
    // Given: set GET method and statusCode = HTTP_OK
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpGet().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

TEST(HttpCacheStrategyFunctionUnitTest, isInvalidCacheMethod_ReturnsFalse_WhenHeadMethod)
{
    // Given: set HEAD method and statusCode = HTTP_OK
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpHead().build();
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).setRequest(pRequest).build();

    // When: call isInvalidCacheMethod
    // Then: return false
    EXPECT_FALSE(HttpCacheStrategy::isInvalidCacheMethod(pResponse));
}

// CacheResponse に 「Warning: 1xx」 がある
TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_RemoveWarning1xx_WhenWarning1xxExistsCachedResponse)
{
    // Given: set Warning 1xx
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderWarning, Warning110Value).
            setHeader(HeaderWarning, Warning113Value).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: remove Warning:1xx
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderWarning) == 0) {
            EXPECT_NE(0, Poco::icompare(it->second, 1, "1"));
        }
    }
}

// CacheResponse に 「Warning: 299 - xxxxx」 がある
TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_ExistsWarning2xx_WhenWarning2xxExistsCachedResponse)
{
    // Given: set Warning 2xx
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderWarning, Warning2xxValue).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: exists Warning:2xx
    bool warning2xxExists = false;
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderWarning) == 0) {
            if (Poco::icompare(it->second, Warning2xxValue) == 0) {
                warning2xxExists = true;
            }
        }
    }
    EXPECT_TRUE(warning2xxExists);
}

// CacheResponse に 「Date:xxx」 があり、NetworkResponse に「Date」がない
TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_UsesDateFromCachedResponse_WhenDateExistsOnlyInCachedResponse)
{
    // Given: set Date to cached response.
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderDate, DateValue1).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: use Date of Cached Response
    bool date1Exists = false;
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderDate) == 0) {
            if (Poco::icompare(it->second, DateValue1) == 0) {
                date1Exists = true;
            }
        }
    }
    EXPECT_TRUE(date1Exists);
}

TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_UsesDateFromNetworkResponse_WhenDateExistsInCachedResponseAndNetworkResponse)
{
    // Given: set Date to cached response and network response.
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderDate, DateValue1).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderDate, DateValue2).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: use Date of Network Response
    bool date2Exists = false;
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderDate) == 0) {
            EXPECT_NE(0, Poco::icompare(it->second, DateValue1));
            if (Poco::icompare(it->second, DateValue2) == 0) {
                date2Exists = true;
            }
        }
    }
    EXPECT_TRUE(date2Exists);
}

// CacheResponse に 「Connection: Close」 があり、NetworkResponse に「Connection: Upgrade」 がある
// (Hop-by-hop Headers は、Cache response を使用)
TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_UsesConnectionFromCachedResponse_WhenConnectionExistsInCachedResponseAndNetworkResponse)
{
    // Given: set Connection to cached response and network response.
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderConnection, ConnectionValue1).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderConnection, ConnectionValue2).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: use Connection of Cached Response
    bool connection1Exists = false;
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderConnection) == 0) {
            EXPECT_NE(0, Poco::icompare(it->second, ConnectionValue2));
            if (Poco::icompare(it->second, ConnectionValue1) == 0) {
                connection1Exists = true;
            }
        }
    }
    EXPECT_TRUE(connection1Exists);
}

// CacheRepone に 「Connection」がなく、NetworkResponse に「Connection:Upgrade」がある
// (Hop-by-hop Headers は、Cache response を使用)
TEST(HttpCacheStrategyFunctionUnitTest,
        combineCacheAndNetworkHeader_NotUseConnectionOfNetworkResponse_WhenConnectionExistsOnlyNetworkResponse)
{
    // Given: set Connection to network response.
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpPost().build();

    Response::Builder cachedResponseBuilder;
    Response::Ptr pCachedResponse = cachedResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).build();

    Response::Builder networkResponseBuilder;
    Response::Ptr pNetworkResponse = networkResponseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_OK).
            setRequest(pRequest).setHeader(HeaderConnection, ConnectionValue2).build();

    // When:: call combineCacheAndNetworkHeader
    Headers::Ptr pHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCachedResponse, pNetworkResponse);

    // Then: not exist Connection
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        EXPECT_NE(0, Poco::icompare(it->first, HeaderConnection));
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
