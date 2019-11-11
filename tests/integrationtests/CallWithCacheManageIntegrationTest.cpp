/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Path.h"
#include "Poco/String.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CommonMacros.h"
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
#include "HttpTestServer.h"
#include "MockInterceptor.h"
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

static const std::string Tag = "CallWithCacheManageIntegrationTest";
static const size_t CacheMaxSizeForLruCheck = 300;
static const char* const DefaultResponseBody = "response data 1";

static const char* const DefaultResponseContentType = "text/plain";

static const char* const LruQuery1 = "test=1";
static const char* const LruQuery2 = "test=2";
static const char* const LruQuery3 = "test=3";
static const char* const LruQuery4 = "test=4";
static const char* const LruQuery5 = "test=5";
static const char* const LruQuery6 = "test=6";
static const char* const LruQuery7 = "test=7";
static const char* const LruQuery8 = "test=8";

#ifndef _WIN32
static const char* const TestDataForCacheFromDb = "/HttpIntegrationTest/01_cache_from_db/HttpCache/unix/cache";
#else
static const char* const TestDataForCacheFromDb = "/HttpIntegrationTest/01_cache_from_db/HttpCache/windows/cache";
#endif

class CallWithCacheManageIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

namespace {

class Body100BytesRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        size_t bodySize = 100;
        response.setContentType(DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(bodySize);

        Poco::URI uri(request.getURI());
        std::string query = uri.getQuery();
        Poco::Buffer<char> body(bodySize);
        memcpy(body.begin(), query.c_str(), query.length());
        for (size_t i = query.length(); i < bodySize; i++) {
            body[i] = '0' + (i % 10);
        }
        std::ostream& ostr = response.send();
        ostr.write(body.begin(), bodySize);
    }
};

} /* namespace */

TEST_F(CallWithCacheManageIntegrationTest, execute_RemovesOldCacheData_WhenGetMethodAndCacheSizeOver)
{
    // Given: create cache until cacheMaxSize

    HttpTestServer testServer;
    Body100BytesRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), CacheMaxSizeForLruCheck);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 1: totalSize = 100
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // request 2: totalSize = 200
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // request 3: totalSize = 300
    Request::Builder requestBuilder3;
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    // check database
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    ASSERT_FALSE(db.getMetadataAll(key1).isNull());
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    ASSERT_FALSE(db.getMetadataAll(key2).isNull());
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    ASSERT_FALSE(db.getMetadataAll(key3).isNull());

    // request 4: totalSize = 400, oldest request is removed.
    Request::Builder requestBuilder4;
    std::string url4 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery4);
    Request::Ptr pRequest4 = requestBuilder3.setUrl(url4).build();
    Call::Ptr pCall4 = pHttpClient->newCall(pRequest4);

    // When: execute
    Response::Ptr pResponse4 = pCall4->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse4->getCode());
    // read response body and close
    std::string responseBody4 = pResponse4->getBody()->toString();

    // Then: request1(oldest request) is removed.
    EXPECT_TRUE(db.getMetadataAll(key1).isNull());
    EXPECT_FALSE(db.getMetadataAll(key2).isNull());
    EXPECT_FALSE(db.getMetadataAll(key3).isNull());
    std::string key4 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url4);
    EXPECT_FALSE(db.getMetadataAll(key4).isNull());

    // check cached file
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
    EXPECT_FALSE(responseBodyFile1.exists());
    Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
    EXPECT_TRUE(responseBodyFile2.exists());
    Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
    EXPECT_TRUE(responseBodyFile3.exists());
    Poco::File responseBodyFile4(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url4));
    EXPECT_EQ(100, responseBodyFile4.getSize());
}

TEST_F(CallWithCacheManageIntegrationTest,
        execute_RemovesOldCacheData_WhenGetMethodAndCacheSizeOverAfterChangedLruOrderByGetMethod)
{
    // Given: create cache until cacheMaxSize

    HttpTestServer testServer;
    Body100BytesRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), CacheMaxSizeForLruCheck);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 1: totalSize = 100
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // request 2: totalsize = 200
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // request 3: totalSize = 300
    Request::Builder requestBuilder3;
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    // GET request 1 to change LRU. oldest:request2 -> request3 -> newest:request1
    Request::Builder requestBuilder1_2;
    Request::Ptr pRequest1_2 = requestBuilder1_2.setUrl(url1).build();
    Call::Ptr pCall1_2 = pHttpClient->newCall(pRequest1_2);
    Response::Ptr pResponse1_2 = pCall1_2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1_2->getCode());
    // read response body and close
    std::string responseBody1_2 = pResponse1_2->getBody()->toString();

    // check database
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    ASSERT_FALSE(db.getMetadataAll(key1).isNull());
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    ASSERT_FALSE(db.getMetadataAll(key2).isNull());
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    ASSERT_FALSE(db.getMetadataAll(key3).isNull());

    // request 4: tptalSize = 400, oldest request is removed.
    Request::Builder requestBuilder4;
    std::string url4 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery4);
    Request::Ptr pRequest4 = requestBuilder3.setUrl(url4).build();
    Call::Ptr pCall4 = pHttpClient->newCall(pRequest4);

    // When: execute
    Response::Ptr pResponse4 = pCall4->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse4->getCode());
    // read response body and close
    std::string responseBody4 = pResponse4->getBody()->toString();

    // Then: request2 is removed. result LRU order: request3 -> request1 -> request4
    EXPECT_FALSE(db.getMetadataAll(key1).isNull());
    EXPECT_TRUE(db.getMetadataAll(key2).isNull());
    EXPECT_FALSE(db.getMetadataAll(key3).isNull());
    std::string key4 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url4);
    EXPECT_FALSE(db.getMetadataAll(key4).isNull());

    // check cached file
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
    EXPECT_TRUE(responseBodyFile1.exists());
    Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
    EXPECT_FALSE(responseBodyFile2.exists());
    Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
    EXPECT_TRUE(responseBodyFile3.exists());
    Poco::File responseBodyFile4(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url4));
    EXPECT_EQ(100, responseBodyFile4.getSize());
}

TEST_F(CallWithCacheManageIntegrationTest, execute_CreateCacheStrategyAndExecuteLruCache_WhenCallFirst)
{
    // Given: store database

    // prepare test data
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb));
    srcTestData.copyTo(HttpTestUtil::getDefaultCachePath());

    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4

    HttpTestServer testServer;
    Body100BytesRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), CacheMaxSizeForLruCheck);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 5 (this request is forceCache but not exist cache and execute only load database)
    Request::Builder requestBuilder5;
    std::string url5 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery5);
    CacheControl::Ptr pRequestCacheControl5 = CacheControl::createForceCache();
    Request::Ptr pRequest5 = requestBuilder5.setUrl(url5).setCacheControl(pRequestCacheControl5).build();
    Call::Ptr pCall5 = pHttpClient->newCall(pRequest5);

    // When: execute (not store to cache)
    Response::Ptr pResponse5 = pCall5->execute();
    ASSERT_FALSE(pResponse5->isSuccessful());

    // Then: load database
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    std::string url4 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery4);
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    std::string key4 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url4);
    std::string key5 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url5);

    // check database (remain 3, 1, 4)
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    EXPECT_FALSE(db.getMetadataAll(key3).isNull());
    EXPECT_FALSE(db.getMetadataAll(key1).isNull());
    EXPECT_FALSE(db.getMetadataAll(key4).isNull());
    EXPECT_TRUE(db.getMetadataAll(key5).isNull());

    // check cached response body
    Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
    EXPECT_TRUE(responseBodyFile3.exists());
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
    EXPECT_TRUE(responseBodyFile1.exists());
    Poco::File responseBodyFile4(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url4));
    EXPECT_TRUE(responseBodyFile4.exists());

    // request 6 and remove 3 and remain 1, 4, 6
    Request::Builder requestBuilder6;
    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery6);
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).build();
    Call::Ptr pCall6 = pHttpClient->newCall(pRequest6);
    Response::Ptr pResponse6 = pCall6->execute();
    // read response body and close
    std::string responseBody6 = pResponse6->getBody()->toString();

    // check database
    EXPECT_TRUE(db.getMetadataAll(key3).isNull());
    EXPECT_FALSE(db.getMetadataAll(key1).isNull());
    EXPECT_FALSE(db.getMetadataAll(key4).isNull());
    std::string key6 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url6);
    EXPECT_FALSE(db.getMetadataAll(key6).isNull());

    // check cached response body
    EXPECT_FALSE(responseBodyFile3.exists());
    EXPECT_TRUE(responseBodyFile1.exists());
    EXPECT_TRUE(responseBodyFile4.exists());
    Poco::File responseBodyFile6(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url6));
    EXPECT_TRUE(responseBodyFile6.exists());

    // request 7 and remove 1 remain 4, 6, 7
    Request::Builder requestBuilder7;
    std::string url7 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery7);
    Request::Ptr pRequest7 = requestBuilder7.setUrl(url7).build();
    Call::Ptr pCall7 = pHttpClient->newCall(pRequest7);
    Response::Ptr pResponse7 = pCall7->execute();
    // read response body and close
    std::string responseBody7 = pResponse7->getBody()->toString();

    // check database
    EXPECT_TRUE(db.getMetadataAll(key3).isNull());
    EXPECT_TRUE(db.getMetadataAll(key1).isNull());
    EXPECT_FALSE(db.getMetadataAll(key4).isNull());
    EXPECT_FALSE(db.getMetadataAll(key6).isNull());
    std::string key7 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url7);
    EXPECT_FALSE(db.getMetadataAll(key7).isNull());

    // check cached response body
    EXPECT_FALSE(responseBodyFile3.exists());
    EXPECT_FALSE(responseBodyFile1.exists());
    EXPECT_TRUE(responseBodyFile4.exists());
    EXPECT_TRUE(responseBodyFile6.exists());
    Poco::File responseBodyFile7(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url7));
    EXPECT_TRUE(responseBodyFile7.exists());

    // request 8 and remove 4 and remain 6, 7, 8
    Request::Builder requestBuilder8;
    std::string url8 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery8);
    Request::Ptr pRequest8 = requestBuilder8.setUrl(url8).build();
    Call::Ptr pCall8 = pHttpClient->newCall(pRequest8);
    Response::Ptr pResponse8 = pCall8->execute();
    // read response body and close
    std::string responseBody8 = pResponse8->getBody()->toString();

    // check database
    EXPECT_TRUE(db.getMetadataAll(key3).isNull());
    EXPECT_TRUE(db.getMetadataAll(key1).isNull());
    EXPECT_TRUE(db.getMetadataAll(key4).isNull());
    EXPECT_FALSE(db.getMetadataAll(key6).isNull());
    EXPECT_FALSE(db.getMetadataAll(key7).isNull());
    std::string key8 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url8);
    EXPECT_FALSE(db.getMetadataAll(key8).isNull());

    // check cached response body
    EXPECT_FALSE(responseBodyFile3.exists());
    EXPECT_FALSE(responseBodyFile1.exists());
    EXPECT_FALSE(responseBodyFile4.exists());
    EXPECT_TRUE(responseBodyFile6.exists());
    EXPECT_TRUE(responseBodyFile7.exists());
    Poco::File responseBodyFile8(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url8));
    EXPECT_TRUE(responseBodyFile8.exists());
}

// キャッシュread中にpurgeを呼ぶと、Windowsではsharing violationが発生するためEXCLUDE
TEST_F(CallWithCacheManageIntegrationTest, execute_ReturnsResponse_WhenGetAfterPurgeDuringReadFromCache)
{
    // Given: purge during read from cache.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // 1st. create cache

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // 2nd. GET same url and use Cache Response

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // 3rd. purge before close response body stream
    pCache->evictAll();

    // 4th. GET same url and create cache

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).build();
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    // When: execute GET method after purge
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());

    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    // Then: create cache
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key).isNull());

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(DefaultResponseBody, strlen(DefaultResponseBody)));
}

TEST_F(CallWithCacheManageIntegrationTest, execute_ReturnsRespnseAndCreateCache_WhenGetAfterDeleteCacheDirectory)
{
    // Given: delete cache directory and cache lru strategy is kept in memory.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // 1st. create cache

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // 2nd. delete cache directory
    // call HttpCache::evictAll() to avoid occurring sharing violation in windows.
    // if not call evictAll(), sharing violation will occur when call cacheRootDir.remove()
    pCache->evictAll();
    Poco::File cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    cacheRootDir.remove(true);

    // 3nd. GET same url and access to network

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // check network access
    EXPECT_TRUE(pResponse2->getCacheResponse().isNull());
    EXPECT_FALSE(pResponse2->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    Poco::Timestamp endTime2;

    // Then: create Cache
    std::time_t startSec2 = startTime2.epochTime();
    std::time_t endSec2 = endTime2.epochTime();
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata2;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    pMetadata2 = db.getMetadataAll(key);
    ASSERT_FALSE(pMetadata2.isNull());
    EXPECT_THAT(pMetadata2->getSentRequestAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(pMetadata2->getReceivedResponseAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(pMetadata2->getCreatedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));
    EXPECT_THAT(pMetadata2->getLastAccessedAtEpoch(), testutil::isTimeInRange(startSec2, endSec2));

    // check cached response body
    EXPECT_THAT(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet, url),
            testutil::equalsContentsOfFile(DefaultResponseBody, strlen(DefaultResponseBody)));

    // 4th. GET same url (use cached response)

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    Poco::Timestamp startTime3;

    // execute GET method
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());

    // use cached response
    EXPECT_FALSE(pResponse3->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse3->getNetworkResponse().isNull());

    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    Poco::Timestamp endTime3;

    // not replace cache.
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata3;
    pMetadata3 = db.getMetadataAll(key);
    ASSERT_FALSE(pMetadata3.isNull());
    EXPECT_EQ(pMetadata2->getSentRequestAtEpoch(), pMetadata3->getSentRequestAtEpoch());
    EXPECT_EQ(pMetadata2->getReceivedResponseAtEpoch(), pMetadata3->getReceivedResponseAtEpoch());
    EXPECT_EQ(pMetadata2->getCreatedAtEpoch(), pMetadata3->getCreatedAtEpoch());
    EXPECT_THAT(pMetadata3->getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime3.epochTime(),
            endTime3.epochTime()));
}

} /* namespace test */
} /* namespace easyhttpcpp */
