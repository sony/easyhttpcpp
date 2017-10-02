/*
 * Copyright 2017 Sony Corporation
 */

#include <string>
#include <Poco/Net/HTTPResponse.h>

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
#include "FileContentsEqualMatcher.h"
#include "HeaderContainMatcher.h"
#include "HeadersEqualMatcher.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "TestDefs.h"
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

static const std::string Tag = "CallWithCacheAfterSendRequestIntegrationTest";

static const char* const HttpStatusMessageOk = "OK";
static const char* const HeaderValueNoStore = "no-store";

static const char* const DifferentResponseBody1 = "different response body 1";
static const char* const DifferentResponseContentType1 = "text/plain";

static const char* const DifferentResponseBody2 = "<html><body>different response body 2</body><html>";
static const char* const DifferentResponseContentType2 = "text/html";

static const size_t ResponseBufferBytes = 8192;

class CallWithCacheAfterSendRequestIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
};

namespace {

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

class DifferentResponseBodyRequestHandler1st : public Poco::Net::HTTPRequestHandler {
public:

    DifferentResponseBodyRequestHandler1st()
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        Poco::URI uri(request.getURI());
        std::string query = uri.getQuery();
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        std::string responseBody;
        response.setContentType(DifferentResponseContentType1);
        responseBody = DifferentResponseBody1;
        response.setContentLength(responseBody.length());

        std::ostream& ostr = response.send();
        ostr << responseBody;
    }
};

class DifferentResponseBodyRequestHandler2nd : public Poco::Net::HTTPRequestHandler {
public:

    DifferentResponseBodyRequestHandler2nd()
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        Poco::URI uri(request.getURI());
        std::string query = uri.getQuery();
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        std::string responseBody;
        response.setContentType(DifferentResponseContentType2);
        responseBody = DifferentResponseBody2;
        response.setContentLength(responseBody.length());

        std::ostream& ostr = response.send();
        ostr << responseBody;
    }
};

class NoStoreResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
        response.set(HttpTestConstants::HeaderCacheControl, HeaderValueNoStore);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class TransferEncodingIsChunkedWithLastModifiedResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.set(HttpTestConstants::HeaderLastModified, HttpTestConstants::HeaderValueLastModified);
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setTransferEncoding(Poco::Net::HTTPServerResponse::CHUNKED_TRANSFER_ENCODING);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::Chunked01ResponseBody;
        ostr.flush();
        ostr << HttpTestConstants::Chunked02ResponseBody;
    }
};

class NotModifiedResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
        response.send();
    }
};

class NoContentLengthResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class ContentLengthIsMinusOneResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(-1);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

class OneHourMaxAgeAndSecondIsDefferentResponseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    OneHourMaxAgeAndSecondIsDefferentResponseRequestHandler() : m_count(0)
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        if (m_count == 0) {
            m_count++;
            response.setContentType(HttpTestConstants::DefaultResponseContentType);
            response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
            response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);
            std::ostream& ostr = response.send();
            ostr << HttpTestConstants::DefaultResponseBody;
        } else {
            response.setContentType(DifferentResponseContentType2);
            response.setContentLength(strlen(DifferentResponseBody2));
            std::ostream& ostr = response.send();
            ostr << DifferentResponseBody2;
        }
    }
private:
    int m_count;
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

class ContentLengthZeroWithMaxAgeOneHourResponesRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    ContentLengthZeroWithMaxAgeOneHourResponesRequestHandler(Poco::Net::HTTPResponse::HTTPStatus status) :
            m_status(status)
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(m_status);
        response.setContentLength(0);
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);
        response.send(); // no response body
    }

private:
    const Poco::Net::HTTPResponse::HTTPStatus m_status;
};

class ContentLengthZeroWithLastModifiedResponesRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    ContentLengthZeroWithLastModifiedResponesRequestHandler(Poco::Net::HTTPResponse::HTTPStatus status) :
            m_status(status)
    {
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(m_status);
        response.setContentLength(0);
        response.set(HttpTestConstants::HeaderLastModified, HttpTestConstants::HeaderValueLastModified);
        response.send(); // no response body
    }

private:
    const Poco::Net::HTTPResponse::HTTPStatus m_status;
};

} /* namespace */

TEST_F(CallWithCacheAfterSendRequestIntegrationTest, execute_ReturnsResponseAndStoresToCache_WhenGetMethod)
{
    // Given: not exist in cache, handler set valid Content-Length.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Poco::Timestamp startTime;

    // When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    Poco::Timestamp endTime;

    // Then: store to cache
    // check database
    std::time_t startSec = startTime.epochTime();
    std::time_t endSec = endTime.epochTime();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata));
    EXPECT_EQ(key, metadata.getKey());
    EXPECT_EQ(url, metadata.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata.getStatusMessage().c_str());
    EXPECT_EQ(4, metadata.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Content-Length", "15"), testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), metadata.getResponseBodySize());
    EXPECT_THAT(metadata.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getCreatedAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec, endSec));

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndStoresToCache_WhenGetMethodAndTransferEncodingChunkExistInResponseHeader)
{
    // Given: not exist in cache, handler not set Content-Length and Transfer-Encoding == chunked.

    HttpTestServer testServer;
    TransferEncodingIsChunkedWithLastModifiedResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Poco::Timestamp startTime;

    // When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    Poco::Timestamp endTime;

    // Then: store to cache
    std::string responseBodyData = std::string(HttpTestConstants::Chunked01ResponseBody) +
            HttpTestConstants::Chunked02ResponseBody;
    // check database
    std::time_t startSec = startTime.epochTime();
    std::time_t endSec = endTime.epochTime();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata));
    EXPECT_EQ(key, metadata.getKey());
    EXPECT_EQ(url, metadata.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata.getStatusMessage().c_str());
    EXPECT_EQ(5, metadata.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::containsInHeader("Transfer-Encoding", "chunked"),
        testutil::containsInHeader("Last-Modified", "Mon, 25 Jul 2016 10:13:43 GMT"),
        testutil::hasKeyInHeader("Date")));

    EXPECT_EQ(responseBodyData.size(), metadata.getResponseBodySize());
    EXPECT_THAT(metadata.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getCreatedAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec, endSec));

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(responseBodyData.c_str(), responseBodyData.size()));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndStoresToCache_WhenGetMethodAndStatusCodeIsNotFound)
{
    // Given: use NotFound handler

    HttpTestServer testServer;
    OneHourMaxAgeAndNotFoundRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Poco::Timestamp startTime;

    // When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    Poco::Timestamp endTime;

    // Then: store to cache
    // check database
    std::time_t startSec = startTime.epochTime();
    std::time_t endSec = endTime.epochTime();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata));
    EXPECT_EQ(key, metadata.getKey());
    EXPECT_EQ(url, metadata.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, metadata.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata.getStatusMessage().c_str());
    EXPECT_EQ(5, metadata.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Content-Length", "15"), testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::containsInHeader("Cache-Control", "max-age=3600"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), metadata.getResponseBodySize());
    EXPECT_THAT(metadata.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getCreatedAtEpoch(), testutil::isTimeInRange(startSec, endSec));
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec, endSec));

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotStoreToCache_WhenGetMethodAndNoStoreExistResponseHeader)
{
    // Given: handler set no-store to response header.

    HttpTestServer testServer;
    NoStoreResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    // Then: not store to cache
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key, metadata));

    // not exist cached response body
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotStoreToCache_WhenGetMethodAndNotExistContentLengthInResponse)
{
    // Given: handler not set Content-Length

    HttpTestServer testServer;
    NoContentLengthResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    // Then: not store to cache
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key, metadata));

    // check cached response body
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotStoreToCache_WhenGetMethodAndContentLengthIsMinusOneInResponse)
{
    // Given: handler set -1 to Content-Length.

    HttpTestServer testServer;
    ContentLengthIsMinusOneResponseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    // Then: not store to cache
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key, metadata));

    // check cached response body
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndReplaceCache_WhenGetMethodAndExistCacheAndExecuteNetworkRequest)
{
    // Given: exist in cache

    HttpTestServer testServer;
    DifferentResponseBodyRequestHandler1st handler1st;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1st);
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

    // GET same url
    DifferentResponseBodyRequestHandler2nd handler2nd;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2nd);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

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

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // Then: store new response to cache
    // check database
    std::time_t startSec2 = startTime2.epochTime();
    std::time_t endSec2 = endTime2.epochTime();
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(key, metadata2.getKey());
    EXPECT_EQ(url, metadata2.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata2.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata2.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata2.getStatusMessage().c_str());
    EXPECT_EQ(4, metadata2.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata2.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Content-Length", "50"), testutil::containsInHeader("Content-Type", "text/html"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(strlen(DifferentResponseBody2), metadata2.getResponseBodySize());
    EXPECT_THAT(metadata2.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getCreatedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(DifferentResponseBody2, strlen(DifferentResponseBody2)));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseWithCachedResponse_WhenGetMethodAndHttpStatusIsNotModified)
{
    // Given: set handler to return NotModified when conditional request.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler1st handler1st;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1st);
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

    // GET same url
    HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler2nd handler2nd;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2nd);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method. and receive NotModified
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED, pResponse2->getNetworkResponse()->getCode());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // check database
    std::time_t startSec2 = startTime2.epochTime();
    std::time_t endSec2 = endTime2.epochTime();
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(key, metadata2.getKey());
    EXPECT_EQ(url, metadata2.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata2.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata2.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata2.getStatusMessage().c_str());
    EXPECT_EQ(6, metadata2.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata2.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Content-Length", "15"), testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::containsInHeader("Last-Modified", "Mon, 25 Jul 2016 10:13:43 GMT"),
        testutil::containsInHeader("Cache-Control", "max-age=3600"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), metadata2.getResponseBodySize());
    EXPECT_THAT(metadata2.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getCreatedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));

    // check cache response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotStoreToCache_WhenGetMethodAndAfterRequestThatHttpStatusIsNotModified)
{
    // Given: first request create cache.
    //        second request receive Not Modified and receive from cache

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler1st handler1st;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1st);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // first request

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

    // second request

    // GET same url
    HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler2nd handler2nd;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2nd);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method. and receive NotModified
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // third request

    // GET same url (not access network)
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    // When: execute GET method. and not access network
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());

    // Then: receive from cache.
    EXPECT_FALSE(pResponse3->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse3->getNetworkResponse().isNull());

    // read response body
    std::string responseBody3 = pResponse3->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody3);
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndNotStoreToCache_WhenGetMethodAndNoCacheInRequestCacheControl)
{
    // Given: exist in cache

    HttpTestServer testServer;
    OneHourMaxAgeAndSecondIsDefferentResponseRequestHandler handler;
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

    // check response header
    ssize_t contentLength1 = pResponse1->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength1);

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();

    // When: GET Method with no-cache in request
    CacheControl::Builder cacheControlBuilder2;
    CacheControl::Ptr pCacheControl2 = cacheControlBuilder2.setNoCache(true).build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).setCacheControl(pCacheControl2).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: cache not change.

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // check database (old data for no-cache)
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(key, metadata2.getKey());
    EXPECT_EQ(url, metadata2.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata2.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata2.getStatusCode());
    EXPECT_STREQ(HttpStatusMessageOk, metadata2.getStatusMessage().c_str());
    EXPECT_THAT(metadata2.getResponseHeaders(), testutil::equalHeaders(metadata1.getResponseHeaders()));
    EXPECT_EQ(metadata1.getResponseBodySize(), metadata2.getResponseBodySize());
    EXPECT_EQ(metadata1.getSentRequestAtEpoch(), metadata2.getSentRequestAtEpoch());
    EXPECT_EQ(metadata1.getReceivedResponseAtEpoch(), metadata2.getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata1.getCreatedAtEpoch(), metadata2.getCreatedAtEpoch());
    EXPECT_EQ(metadata1.getLastAccessedAtEpoch(), metadata2.getLastAccessedAtEpoch());

    // check cached response body (old data for no-cache)
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest, execute_ReturnesResponseAndNotStoreToCache_WhenPostMethod)
{
    // Given: not exist in cache

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpPost().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    // Then: not store to cache.
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key, metadata));

    // check cached response body
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseAndRemovesFromCache_WhenPostMethodAndExistCache)
{
    // Given: exist in cache

    HttpTestServer testServer;
    DifferentResponseBodyRequestHandler1st handler1st;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1st);
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

    // POST same url
    DifferentResponseBodyRequestHandler2nd handler2nd;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2nd);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.httpPost().setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // When: execute POST method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: remove cached response.

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // not exist in cache
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_FALSE(db.getMetadataAll(key, metadata2));

    // not exist cached response body
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

TEST_F(CallWithCacheAfterSendRequestIntegrationTest,
        execute_ReturnsResponseWithCachedResponse_WhenGetMethodAndTransferEncodingIsChunkedAndHttpStatusIsNotModified)
{
    // Given: first request is Transfer-Encoding:chunked and second request is NotModified with conditional request.

    HttpTestServer testServer;
    TransferEncodingIsChunkedWithLastModifiedResponseRequestHandler handler1st;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1st);
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

    std::string responseBodyData = std::string(HttpTestConstants::Chunked01ResponseBody) +
            HttpTestConstants::Chunked02ResponseBody;
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(responseBodyData.c_str(), responseBodyData.size()));

    // GET same url
    NotModifiedResponseRequestHandler notModfiedHandler;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &notModfiedHandler);

    Interceptor::Ptr pMockNetworkInterceptor2 = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor2.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor2).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    // Then: receive NotModified. response from cache
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED, pResponse2->getNetworkResponse()->getCode());
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(responseBody1, responseBody2);
    EXPECT_FALSE(pResponse2->getBody()->hasContentLength());
    EXPECT_FALSE(pResponse2->hasContentLength());

    Poco::Timestamp endTime2;

    // check database
    std::time_t startSec2 = startTime2.epochTime();
    std::time_t endSec2 = endTime2.epochTime();
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));
    EXPECT_EQ(key, metadata2.getKey());
    EXPECT_EQ(url, metadata2.getUrl());
    EXPECT_EQ(Request::HttpMethodGet, metadata2.getHttpMethod());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, metadata2.getStatusCode());
    EXPECT_EQ(5, metadata2.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata2.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Transfer-Encoding", "chunked"),
        testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::containsInHeader("Last-Modified", "Mon, 25 Jul 2016 10:13:43 GMT"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(responseBodyData.size(), metadata2.getResponseBodySize());
    EXPECT_THAT(metadata2.getSentRequestAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getCreatedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(metadata2.getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));

    // もう１回実行しても、同様に、ネットワークアクセスして、NotModified 受信で、Cacahe を使用する。
    Interceptor::Ptr pMockNetworkInterceptor3 = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor3.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor3).
            build();
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    // execute.
    // receive NotModified. response from cache
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED, pResponse3->getNetworkResponse()->getCode());
    EXPECT_FALSE(pResponse3->getCacheResponse().isNull());

    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();
    EXPECT_EQ(responseBody1, responseBody3);
    EXPECT_FALSE(pResponse3->getBody()->hasContentLength());
    EXPECT_FALSE(pResponse3->hasContentLength());

    // check database
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    EXPECT_TRUE(db.getMetadataAll(key, metadata3));
    EXPECT_EQ(key, metadata3.getKey());
    EXPECT_EQ(url, metadata3.getUrl());
    EXPECT_EQ(5, metadata3.getResponseHeaders()->getSize());
    EXPECT_THAT(metadata3.getResponseHeaders(), testing::AllOf(testutil::containsInHeader("Connection", "Keep-Alive"),
        testutil::containsInHeader("Transfer-Encoding", "chunked"),
        testutil::containsInHeader("Content-Type", "text/plain"),
        testutil::containsInHeader("Last-Modified", "Mon, 25 Jul 2016 10:13:43 GMT"),
        testutil::hasKeyInHeader("Date")));
    EXPECT_EQ(responseBodyData.size(), metadata3.getResponseBodySize());
}

namespace {

class HttpStatusParam {
public:
    const Poco::Net::HTTPResponse::HTTPStatus status;
    std::string print() const
    {
        std::string ret = std::string("\n") +
                "status : " + StringUtil::format("%d", status) + "\n";
        return ret;
    }
};

} /* namespace */

static const HttpStatusParam ContentLengthZreoHttpStatusData[] = {
    {
        Poco::Net::HTTPResponse::HTTP_OK
    },
    {
        Poco::Net::HTTPResponse::HTTP_NO_CONTENT
    }
};

class ContentLengthZeroParameterisedTest : public CallWithCacheAfterSendRequestIntegrationTest,
        public testing::WithParamInterface<HttpStatusParam> {
};
INSTANTIATE_TEST_CASE_P(CallWithCacheAfterSendRequestIntegrationTest, ContentLengthZeroParameterisedTest,
        testing::ValuesIn(ContentLengthZreoHttpStatusData));

TEST_P(ContentLengthZeroParameterisedTest,
        execute_StoresToCache_WhenContentLengthIsZeroAndDoNotReadResponseBodyStream)
{
    HttpStatusParam& param = (HttpStatusParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given: use cache, http handler return content-length:0
    HttpTestServer testServer;
    ContentLengthZeroWithMaxAgeOneHourResponesRequestHandler handler(param.status);
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

    // When: execute GET method and receive Content-Length=0 of Response and close response body without read.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(param.status, pResponse1->getCode());
    EXPECT_TRUE(pResponse1->hasContentLength());
    EXPECT_EQ(0, pResponse1->getContentLength());
    pResponse1->getBody()->close();

    // Then: store to cache
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));
    EXPECT_EQ(0, metadata1.getResponseBodySize());

    // check cached response body
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_TRUE(responseBodyFile1.exists());
    EXPECT_EQ(0, responseBodyFile1.getSize());

    // The GET of the same url returns the response of cache.
    // response body is 0 bytes.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(param.status, pResponse2->getCode());
    EXPECT_TRUE(pResponse2->hasContentLength());
    EXPECT_EQ(0, pResponse2->getContentLength());

    // check cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse2->getNetworkResponse().isNull());

    // read response body.
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponse2->getBody()->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    EXPECT_EQ(0, HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2));
    pResponseBodyStream2->close();
}

TEST_P(ContentLengthZeroParameterisedTest,
        execute_ReturnsResponseFromCache_WhenExistCacheOfContentLengthIsZeroWithNotReadResponseBodyStreamAndConditionalRequestReceiveNotModified)
{
    HttpStatusParam& param = (HttpStatusParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given: use cache, http handler return content-length:0
    HttpTestServer testServer;
    ContentLengthZeroWithLastModifiedResponesRequestHandler handler(param.status);
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

    // execute GET method and receive Content-Length=0 of Response and close response body without read.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(param.status, pResponse1->getCode());
    EXPECT_TRUE(pResponse1->hasContentLength());
    EXPECT_EQ(0, pResponse1->getContentLength());
    pResponse1->getBody()->close();

    // store to cache
    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));
    EXPECT_EQ(0, metadata1.getResponseBodySize());

    // check cached response body
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_TRUE(responseBodyFile1.exists());
    EXPECT_EQ(0, responseBodyFile1.getSize());

    // change handler
    NotModifiedResponseRequestHandler notModfiedHandler;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &notModfiedHandler);

    // When: The GET of the same url.
    // Then: returns the response of cache after conditional request and receive NotModified.
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(param.status, pResponse2->getCode());
    EXPECT_TRUE(pResponse2->hasContentLength());
    EXPECT_EQ(0, pResponse2->getContentLength());

    // check cached response.
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    ASSERT_FALSE(pResponse2->getNetworkResponse().isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED, pResponse2->getNetworkResponse()->getCode());

    // read response body.
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponse2->getBody()->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    EXPECT_EQ(0, HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2));
    pResponseBodyStream2->close();
}

} /* namespace test */
} /* namespace easyhttpcpp */
