/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/DirectoryIterator.h"
#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Path.h"
#include "Poco/String.h"
#include "Poco/SharedPtr.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CommonException.h"
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
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"
#include "TestFileUtil.h"
#include "TestLogger.h"

#include "HttpCacheDatabase.h"
#include "HttpCacheInternal.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::PocoException;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;
using easyhttpcpp::testutil::TestFileUtil;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "HttpCacheIntegrationTest";
static const char* const LruQuery1 = "test=1";
static const char* const LruQuery2 = "test=2";
static const char* const LruQuery3 = "test=3";
static const char* const LruQuery4 = "test=4";
static const size_t ResponseBufferBytes = 8192;
#ifndef _WIN32
static const char* const TestDataForCacheFromDb = "/HttpIntegrationTest/01_cache_from_db/HttpCache/unix/cache";
#else
static const char* const TestDataForCacheFromDb = "/HttpIntegrationTest/01_cache_from_db/HttpCache/windows/cache";
#endif
static const char* const TempDummyFileName = "dummy";

class HttpCacheIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

// evictAll
// 1. CacheのresponseBodyが削除される。
// 2. CacheのDatabase が空になる。
// 3. temp directory が削除される。
TEST_F(HttpCacheIntegrationTest, evictAll_DeletesAllCache)
{
    // Given: create cache in some url.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 1
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // request 2
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // request 3
    Request::Builder requestBuilder3;
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    // read response body and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    EXPECT_TRUE(db.getMetadataAll(key1, metadata1));
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    EXPECT_TRUE(db.getMetadataAll(key2, metadata2));
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    EXPECT_TRUE(db.getMetadataAll(key3, metadata3));

    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile1.getSize());
    Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile2.getSize());
    Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile3.getSize());

    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    ASSERT_TRUE(tempDir.exists());

    // When: call evictAll
    pCache->evictAll();

    // Then: remove all cache
    EXPECT_FALSE(db.getMetadataAll(key1, metadata1));
    EXPECT_FALSE(db.getMetadataAll(key2, metadata2));
    EXPECT_FALSE(db.getMetadataAll(key3, metadata3));

    EXPECT_FALSE(responseBodyFile1.exists());
    EXPECT_FALSE(responseBodyFile2.exists());
    EXPECT_FALSE(responseBodyFile3.exists());
    
    EXPECT_FALSE(tempDir.exists());
}

// response read 中の evictAll
// 1. CacheのresponseBodyが削除される。
// 2. CacheのDatabase が空になる。
// 3. temp directory が削除される。
TEST_F(HttpCacheIntegrationTest, evictAll_DeletesAllCache_WhenExistReadingResponse)
{
    // Given: create cache and temp file is reading.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 1
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // request 2
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // request 3 reading not complete
    Request::Builder requestBuilder3;
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());

    ResponseBody::Ptr pResponseBody3 = pResponse3->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream3 = pResponseBody3->getByteStream();
    Poco::Buffer<char> responseBodyBuffer3(strlen(HttpTestConstants::DefaultResponseBody));
    EXPECT_EQ(10, pResponseBodyStream3->read(responseBodyBuffer3.begin(), 10));

    // confirm database (key3 is not exist in cache))
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    EXPECT_TRUE(db.getMetadataAll(key1, metadata1));
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    EXPECT_TRUE(db.getMetadataAll(key2, metadata2));
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    EXPECT_FALSE(db.getMetadataAll(key3, metadata3));

    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile1.getSize());
    Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile2.getSize());
    Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
    EXPECT_FALSE(responseBodyFile3.exists());

    // temp file exist. (key3)
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    ASSERT_TRUE(tempDir.exists());
    Poco::DirectoryIterator itDir(tempDir);
    Poco::DirectoryIterator itEnd;
    ASSERT_NE(itEnd, itDir);

    // When: evictAll
    pCache->evictAll();

    // Then: delete all cache
    EXPECT_FALSE(db.getMetadataAll(key1, metadata1));
    EXPECT_FALSE(db.getMetadataAll(key2, metadata2));
    EXPECT_FALSE(db.getMetadataAll(key3, metadata3));

    EXPECT_FALSE(responseBodyFile1.exists());
    EXPECT_FALSE(responseBodyFile2.exists());
    EXPECT_FALSE(responseBodyFile3.exists());

    EXPECT_FALSE(tempDir.exists());
}

TEST_F(HttpCacheIntegrationTest, evictAll_DeletesAllCache_WhenBeforeFirstCacheAccess)
{
    // Given: load test database
    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb);
    srcTestData.copyTo(cachePath);

    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // When: evictAll
    pCache->evictAll();

    // Then: delete all cache
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    EXPECT_FALSE(db.getMetadataAll(key1, metadata1));
   HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    std::string url3 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery3);
    std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
    EXPECT_FALSE(db.getMetadataAll(key3, metadata3));
    HttpCacheDatabase::HttpCacheMetadataAll metadata4;
    std::string url4 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery4);
    std::string key4 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url4);
    EXPECT_FALSE(db.getMetadataAll(key4, metadata1));
}

// CacheManager::purge が失敗すると、evictAll が HttpExecutionException を throw する。
TEST_F(HttpCacheIntegrationTest,
        evictAll_ThrowsHttpExecutionException_WhenPurgeOfCacheManagerReturnedFalse)
{
    // Given: response を cache に格納し、CacheManager::purge を失敗させるため、Database を read only にする。
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // response を cache に格納する。
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    Response::Ptr pResponse = pCall->execute();
    std::string responseBody = pResponse->getBody()->toString();

    // Database を read only にする。
    TestFileUtil::changeAccessPermission(HttpTestUtil::getDefaultCacheDatabaseFile(),
            EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY);

    // When: call evictAll
    // Then: HttpExecutionException (cause なし) が throw される。
    EASYHTTPCPP_EXPECT_THROW(pCache->evictAll(), HttpExecutionException, 100702);
}

// Cache の temporary directory の削除が失敗すると、evictAll が HttpExecutionException を throw する。
TEST_F(HttpCacheIntegrationTest, evictAll_ThrowsHttpExecutionException_WhenFailedToDeleteCacheTempDir)
{
    // Given: response を cache に格納し、temporary directory の削除を失敗させるため、
    // temporary directory に dummy の file を置いて、temporary directory を read only にする。
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // response を cache に格納する。
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    Response::Ptr pResponse = pCall->execute();
    std::string responseBody = pResponse->getBody()->toString();

    // dummy file を置いて、Cache temporary directory を read only にする。
    Poco::File dummyFile(HttpTestUtil::getDefaultCacheTempDir() + TempDummyFileName);
    dummyFile.createFile();
    TestFileUtil::changeAccessPermission(HttpTestUtil::getDefaultCacheTempDir(),
            EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY);

    // When: call evictAll
    // Then: HttpExecutionException (cause なし) が throw される。
    EASYHTTPCPP_EXPECT_THROW(pCache->evictAll(), HttpExecutionException, 100702);

    TestFileUtil::changeAccessPermission(HttpTestUtil::getDefaultCacheTempDir(),
            EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS);
}

// cache にないurl で呼び出し
// CachedResponse が NULL の HttpCacheStrategy が作成される。
TEST_F(HttpCacheIntegrationTest, createCacheStrategy_NotSetsCachedResponse_WhenNotExistCache)
{
    // Given: create no cached url request
    Poco::Path path(HttpTestUtil::getDefaultCachePath());
    HttpCache::Ptr pCache = HttpCache::createCache(path, 0);

    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();

    // When: call cacheStrategy
    HttpCacheInternal* pHttpCacheInternal = static_cast<HttpCacheInternal*>(pCache.get());
    HttpCacheStrategy::Ptr pCacheStrategy = pHttpCacheInternal->createCacheStrategy(pRequest);

    // Then: cached response is none. network request exist.
    EXPECT_TRUE(pCacheStrategy->getCachedResponse().isNull());
    EXPECT_FALSE(pCacheStrategy->getNetworkRequest().isNull());
}

// cache にあるurl
// CachedResponse が格納された HttpCacheStrategy が作成される。
TEST_F(HttpCacheIntegrationTest, createCacheStrategy_SetsCachedResponse_WhenExistCache)
{
    // Given: create cache

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // When: call cacheStrategy
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    HttpCacheInternal* pHttpCacheInternal = static_cast<HttpCacheInternal*>(pCache.get());
    HttpCacheStrategy::Ptr pCacheStrategy = pHttpCacheInternal->createCacheStrategy(pRequest2);

    // Then: cached response make from cached response metadata.
    Response::Ptr pCachedResponse = pCacheStrategy->getCachedResponse();
    EXPECT_FALSE(pCachedResponse.isNull());
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata));
    EXPECT_EQ(url, pCachedResponse->getRequest()->getUrl());
    EXPECT_EQ(metadata.getStatusCode(), pCachedResponse->getCode());
    EXPECT_EQ(metadata.getStatusMessage(), pCachedResponse->getMessage());
    EXPECT_EQ(metadata.getResponseBodySize(), pCachedResponse->getContentLength());
    EXPECT_TRUE(pCachedResponse->getBody().isNull());

    EXPECT_TRUE(pCacheStrategy->getNetworkRequest().isNull());
}

// remove
// Cache から、response body が削除される。
TEST_F(HttpCacheIntegrationTest, remove_RemovesResponseFromCache_WhenExistCache)
{
    // Given: create cache
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check cache database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url));
    EXPECT_TRUE(responseBodyFile.exists());

    // When: call remove
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    HttpCacheInternal* pHttpCacheInternal = static_cast<HttpCacheInternal*>(pCache.get());
    pHttpCacheInternal->remove(pRequest2);

    // Then: remove from cache
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_FALSE(db.getMetadataAll(key, metadata2));
    EXPECT_FALSE(responseBodyFile.exists());
}

// response が cache にある場合
// std::istream が返る
TEST_F(HttpCacheIntegrationTest, createInputStreamFromCache_ReturnsIstream_WhenExistCache)
{
    // Given: exist in cache.

    // set test handler
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check cache database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    ASSERT_TRUE(db.getMetadataAll(key, metadata1));

    // When: call createInputStreamFromCache
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    HttpCacheInternal* pHttpCacheInternal = static_cast<HttpCacheInternal*>(pCache.get());
    Poco::SharedPtr<std::istream> pStream = pHttpCacheInternal->createInputStreamFromCache(pRequest2);

    // Then: get istream
    EXPECT_FALSE(pStream.isNull());
    Poco::Buffer<char> buffer(ResponseBufferBytes);
    pStream->read(buffer.begin(), ResponseBufferBytes);
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), pStream->gcount());
    EXPECT_EQ(0, memcmp(buffer.begin(), HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

// response が cache にない場合
// NULL が返る
TEST_F(HttpCacheIntegrationTest, createInputStreamFromCache_ReturnsNull_WhenNotExistCache)
{
    // Given: not exist in cache.

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // When: call createInputStreamFromCache
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    HttpCacheInternal* pHttpCacheInternal = static_cast<HttpCacheInternal*>(pCache.get());
    Poco::SharedPtr<std::istream> pStream = pHttpCacheInternal->createInputStreamFromCache(pRequest);

    // Then: can not get istream
    EXPECT_TRUE(pStream.isNull());
}

// Cache が空の場合
// 0 が返る。
TEST_F(HttpCacheIntegrationTest, getSize_ReturnsZero_WhenEmptyInCache)
{
    // Given: create empty cache
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // When: call getSize
    size_t cachedSize = pCache->getSize();

    // Then: size is 0
    EXPECT_EQ(0, cachedSize);
}

// Cache に response がある場合
// Cache のresponse body の合計サイズが取得できる。
TEST_F(HttpCacheIntegrationTest, getSize_ReturnsTotalSizeOfCachedResponseBody_WhenNotEmptyInCache)
{
    // Given: exist some response in cache.

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // request 1
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // request 2
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    // read response body and close
    std::string responseBody2 = pResponse2->getBody()->toString();

    // When: call getSize
    size_t cachedSize = pCache->getSize();

    // Then: total response body size
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody) * 2, cachedSize);
}

} /* namespace test */
} /* namespace easyhttpcpp */
