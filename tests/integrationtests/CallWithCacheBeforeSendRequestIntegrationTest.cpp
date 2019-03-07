/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Path.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "HeadersEqualMatcher.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"
#include "TestDefs.h"
#include "TestLogger.h"
#include "TimeInRangeMatcher.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithCacheBeforeSendRequestIntegrationTest";

static const char* const HeaderLastModified = "Last-Modified";
static const char* const HeaderIfModifiedSince = "If-Modified-Since";
static const char* const HeaderIfNoneMatch = "If-None-Match";
static const char* const HeaderEtag = "ETag";
static const char* const HeaderDate = "Date";
static const char* const HeaderExpires = "Expires";

static const char* const HeaderValueEtag = "Tag123456789";
static const char* const HeaderValueLastModified = "Mon, 25 Jul 2016 10:13:43 GMT";
static const char* const HeaderValueDate = "Fri, 22 Jul 2016 15:30:12 GMT";

static const char* const DifferentResponseBody2 = "<html><body>different response body 2</body><html>";
static const char* const DifferentResponseContentType2 = "text/html";

class CallWithCacheBeforeSendRequestIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

namespace {

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

Response::Ptr delegateCheckIfNoneMatchIntercept(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    const std::string& ifNoneMatch = pRequest->getHeaderValue(HeaderIfNoneMatch, "");
    if (Poco::icompare(ifNoneMatch, HeaderValueEtag) == 0) {
        return chain.proceed(chain.getRequest());
    } else {
        Response::Builder builder;
        return builder.setRequest(pRequest).setCode(-1).build();
    }
}

class DelegateIfModifiedSinceInterceptor {
public:
    void setIfModifiedSinceValue(const std::string& ifModifiedSinceValue)
    {
        m_ifModifiedSinceCheckValue = ifModifiedSinceValue;
    }
    Response::Ptr delegateCheckIfModifiedSinceIntercept(Interceptor::Chain& chain)
    {
        Request::Ptr pRequest = chain.getRequest();
        const std::string& ifModifiedSince = pRequest->getHeaderValue(HeaderIfModifiedSince, "");
        if (Poco::icompare(ifModifiedSince, m_ifModifiedSinceCheckValue) == 0) {
            return chain.proceed(chain.getRequest());
        } else {
            Response::Builder builder;
            return builder.setRequest(pRequest).setCode(-1).build();
        }
    }
private:
    std::string m_ifModifiedSinceCheckValue;
};

class ExpiresOneHourRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        Poco::Timespan oneHour(0, 1, 0, 0, 0);  // 1 hour
        Poco::Timestamp expires;
        expires += oneHour;
        response.set(HeaderExpires,
                Poco::DateTimeFormatter::format(expires, Poco::DateTimeFormat::HTTP_FORMAT));

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class TenPercentOfDateToSentRequestTimeIsOneHourRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        Poco::Timespan oneHour(0, 10, 0, 0, 0);  // 10 hour
        Poco::Timestamp lastModified;
        lastModified -= oneHour;
        response.set(HeaderLastModified,
                Poco::DateTimeFormatter::format(lastModified, Poco::DateTimeFormat::HTTP_FORMAT));

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class ExpiresMinusOneAndExistsDateRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderExpires, "-1");
        response.set(HeaderDate, HeaderValueDate);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class ETagResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderEtag, HeaderValueEtag);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class LastModifiedResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderLastModified, HeaderValueLastModified);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class DateResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderDate, HeaderValueDate);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class ETagAndLastModifiedResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderEtag, HeaderValueEtag);
        response.set(HeaderLastModified, HeaderValueLastModified);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class ETagAndDateResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderEtag, HeaderValueEtag);
        response.set(HeaderDate, HeaderValueDate);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class LastModifiedAndDateResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HeaderLastModified, HeaderValueLastModified);
        response.set(HeaderDate, HeaderValueDate);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class DefferentResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    DefferentResponseRequestHandler()
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType(DifferentResponseContentType2);
        response.setContentLength(strlen(DifferentResponseBody2));
        std::ostream& ostr = response.send();
        ostr << DifferentResponseBody2;
    }
};

class OneHourMaxAgeAndNotFoundRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class TransferEncodingIsChunkedAndMaxAgeOneHourResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setTransferEncoding(Poco::Net::HTTPServerResponse::CHUNKED_TRANSFER_ENCODING);
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::Chunked01ResponseBody;
        ostr.flush();
        ostr << HttpTestConstants::Chunked02ResponseBody;
    }
};

} /* namespace */

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUsesCachedResponse_WhenGetMethodAndAgeIsSmallerThanMaxAge)
{
    // Given: max-age is 1 hour. age is in range of max-age.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. without network access
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // database is same as 1st request (except lastAccessedAtEpoch)
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(metadata1.getKey(), metadata2.getKey());
    EXPECT_EQ(metadata1.getUrl(), metadata2.getUrl());
    EXPECT_EQ(metadata1.getHttpMethod(), metadata2.getHttpMethod());
    EXPECT_EQ(metadata1.getStatusCode(), metadata2.getStatusCode());
    EXPECT_EQ(metadata1.getStatusMessage(), metadata2.getStatusMessage());
    EXPECT_THAT(metadata2.getResponseHeaders(), testutil::equalHeaders(metadata1.getResponseHeaders()));
    EXPECT_EQ(metadata1.getResponseBodySize(), metadata2.getResponseBodySize());
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime2.epochTime(),
            endTime2.epochTime()));
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUsesCachedResponse_WhenGetMethodAndAgeIsSmallerThanExpires)
{
    // Given: Expires is 1 hour. age is in range of Expires.

    HttpTestServer testServer;
    ExpiresOneHourRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. without network access
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // database is same as 1st request (except lastAccessedAtEpoch)
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime2.epochTime()
           , endTime2.epochTime()));
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUsesCachedResponse_WhenGetMethodAndAgeIsSmallerThan10percentOfDateToSentRequestTime)
{
    // Given: age is in range of 10% of Last-Modifies to Date.

    HttpTestServer testServer;
    TenPercentOfDateToSentRequestTimeIsOneHourRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. without network access
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // check database
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime2.epochTime(),
            endTime2.epochTime()));
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUsesCachedResponse_WhenGetMethodAndCachedStatusCodeIs404AndAgeIsSmallerThanMaxAge)
{
    // Given: statuc code = 404. max-age is 1 hour. age is in range of max-age.

    HttpTestServer testServer;
    OneHourMaxAgeAndNotFoundRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. without network access
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // database is same as 1st request (except lastAccessedAtEpoch)
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(metadata1.getStatusCode(), metadata2.getStatusCode());
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime2.epochTime(),
            endTime2.epochTime()));
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndExpiresIsMinusOne)
{
    // Given: Expires is -1 and Exist Date (Conditional Request)

    HttpTestServer testServer;
    ExpiresMinusOneAndExistsDateRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url. with conditional request.
    DelegateIfModifiedSinceInterceptor delegateInterceptor;
    // set IfModifiedSince value from Date of response.
    delegateInterceptor.setIfModifiedSinceValue(pResponse1->getHeaderValue(HeaderDate, ""));
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(&delegateInterceptor,
            &DelegateIfModifiedSinceInterceptor::delegateCheckIfModifiedSinceIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithETag)
{
    // Given: exist in cache, not exist Cache-Control and exist ETag in cached response header

    HttpTestServer testServer;
    ETagResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateCheckIfNoneMatchIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithLastModified)
{
    // Given: exist in cache, not exist Cache-Control and exist Last-Modified in cached response header

    HttpTestServer testServer;
    LastModifiedResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    DelegateIfModifiedSinceInterceptor delegateInterceptor;
    // set IfModifiedSince value from LastModified of response.
    delegateInterceptor.setIfModifiedSinceValue(pResponse1->getHeaderValue(HeaderLastModified, ""));
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(&delegateInterceptor,
            &DelegateIfModifiedSinceInterceptor::delegateCheckIfModifiedSinceIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithDate)
{
    // Given: exist in cache, not exist Cache-Control and exist Date in cached response header

    HttpTestServer testServer;
    DateResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    DelegateIfModifiedSinceInterceptor delegateInterceptor;
    // set IfModifiedSince value from Date of response.
    delegateInterceptor.setIfModifiedSinceValue(pResponse1->getHeaderValue(HeaderDate, ""));
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(&delegateInterceptor,
            &DelegateIfModifiedSinceInterceptor::delegateCheckIfModifiedSinceIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithETagAndLastModified)
{
    // Given: exist in cache, not exist Cache-Control and exist ETag and Last-Modified in cached response header

    HttpTestServer testServer;
    ETagAndLastModifiedResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateCheckIfNoneMatchIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithETagAndLastDate)
{
    // Given: exist cache, not exist Cache-Control and exist ETag and Date in cached response header

    HttpTestServer testServer;
    ETagAndDateResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateCheckIfNoneMatchIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndSendsConditionalRequest_WhenGetMethodAndConditionalRequestWithLastModifiedAndDate)
{
    // Given: exist in cache, not exist Cache-Control and exist Last-Modified and Date in cached response header

    HttpTestServer testServer;
    LastModifiedAndDateResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. with conditional request.
    DelegateIfModifiedSinceInterceptor delegateInterceptor;
    // set IfModifiedSince value from LastModified of response.
    delegateInterceptor.setIfModifiedSinceValue(pResponse1->getHeaderValue(HeaderLastModified, ""));
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(&delegateInterceptor,
            &DelegateIfModifiedSinceInterceptor::delegateCheckIfModifiedSinceIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: execute conditional request
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotUseCachedResponse_WhenGetMethodAndIfModifiedSinceInRequestHeader)
{
    // Given: max-age is 1 hour. age is in range of max-age.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerOneHour;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handlerOneHour);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url and network access
    DefferentResponseRequestHandler handlerDefferent;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handlerDefferent);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).setHeader(HeaderIfModifiedSince, HeaderValueLastModified).
            build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: not use cached response.
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotUseCachedResponse_WhenGetMethodAndIfNoneMatchInRequestHeader)
{
    // Given: max-age is 1 hour. age is in range of max-age.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerOneHour;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handlerOneHour);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url and network access
    DefferentResponseRequestHandler handlerDefferent;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handlerDefferent);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).setHeader(HeaderIfNoneMatch, HeaderValueEtag).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: not use cached response.
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndStatucCodeIs504_WhenGetMethodAndForceCacheAndConditionalRequest)
{
    // Given: exist in cache.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url and force cache but need network request.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();

    // When: GET Method and onlyIfCached and no max-age and need network access.
    CacheControl::Builder cacheControlBuilder;
    CacheControl::Ptr pRequestCacheControl2 = cacheControlBuilder.setOnlyIfCached(true).build();

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).setCacheControl(pRequestCacheControl2).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();

    // Then: status code == 504
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_GATEWAY_TIMEOUT, pResponse2->getCode());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUsesCachedResponse_WhenGetMethodAndForceCache)
{
    // Given: exist in cache.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // get database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url with forceCache
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();

    // When: force cache
    CacheControl::Ptr pRequestCacheControl2 = CacheControl::createForceCache();

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).setCacheControl(pRequestCacheControl2).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());
}

// 1. Call::execute、Cache 使用で、ResponseBody2 を read中
// 2. 同じ url をPOST Method でCall::execute
// 3. 同じ url をGet Method でCall::execute すると Network access となる。
TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndRequestByNetwork_WhenGetMethodAfterPostMethodToWhileReadingResponseBody)
{
    // Given: create cache in 1st request and 2nd GET request is reading and 3rd POST request is on going.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // 1st request
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check cache
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // 2nd request is same url and GET Method from cached response.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // use cache
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // 3rd request is POST Method with same url before 2nd request does not finish.
    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).build();
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).httpPost().build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    // execute POST method.
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());

    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    // check cache
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    EXPECT_TRUE(db.getMetadataAll(key, metadata3));

    // 4th request is GET method and same url.
    Interceptor::Ptr pMockNetworkInterceptor4 = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor4.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder4;
    EasyHttp::Ptr pHttpClient4 = httpClientBuilder4.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor4).
            build();
    Request::Builder requestBuilder4;
    Request::Ptr pRequest4 = requestBuilder4.setUrl(url).build();
    Call::Ptr pCall4 = pHttpClient4->newCall(pRequest4);

    // When: execute GET Method
    Response::Ptr pResponse4 = pCall4->execute();
    EXPECT_TRUE(pResponse4->isSuccessful());

    // Then: network access.
    EXPECT_TRUE(pResponse4->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse4->getNetworkResponse().isNull());
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ReturnsResponseAndUseCachedResponse_WhenGetMethodAndTransferEncodingIsChunkedAndAgeIsSmallerThanMaxAge)
{
    // Given: Transfer-Encoding:chunked and max-age is 1 hour. age is in range of max-age.

    HttpTestServer testServer;
    TransferEncodingIsChunkedAndMaxAgeOneHourResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url. without network access
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: use cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(responseBody1, responseBody2);
    EXPECT_FALSE(pResponse2->getBody()->hasContentLength());
    EXPECT_FALSE(pResponse2->hasContentLength());

    Poco::Timestamp endTime2;

    // database is same as 1st request (except lastAccessedAtEpoch)
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(metadata1.getKey(), metadata2.getKey());
    EXPECT_EQ(metadata1.getUrl(), metadata2.getUrl());
    EXPECT_EQ(metadata1.getHttpMethod(), metadata2.getHttpMethod());
    EXPECT_EQ(metadata1.getStatusCode(), metadata2.getStatusCode());
    EXPECT_EQ(metadata1.getStatusMessage(), metadata2.getStatusMessage());
    EXPECT_THAT(metadata2.getResponseHeaders(), testutil::equalHeaders(metadata1.getResponseHeaders()));
    EXPECT_EQ(metadata1.getResponseBodySize(), metadata2.getResponseBodySize());
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime2.epochTime(),
            endTime2.epochTime()));
}

TEST_F(CallWithCacheBeforeSendRequestIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenUseCacheResponseAndCannotMakeResponseBodyStream)
{
    // Given: max-age is 1 hour. age is in range of max-age.
    // responseBodyStream 作成時に Exception を発生させるため、cache file を削除する。

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // cache file を削除する。
    Poco::File cacheFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url));
    ASSERT_TRUE(FileUtil::removeFileIfPresent(cacheFile));

    // GET same url.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute.
    // Then: throw ExecutionException.
    EASYHTTPCPP_EXPECT_THROW(pCall2->execute(), HttpExecutionException, 100702);
}

} /* namespace test */
} /* namespace easyhttpcpp */
