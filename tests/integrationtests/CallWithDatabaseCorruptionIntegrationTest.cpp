/*
 * Copyright 2019 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/File.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

#include "HttpIntegrationTestCase.h"
#include "HttpCacheDatabase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

namespace {

const char* const PathForUrl1 = "/path1";
const char* const PathForUrl2 = "/path2";
const char* const Url1 = "http://localhost:9982/path1";
const char* const Url2 = "http://localhost:9982/path2";

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

Response::Ptr delegateProceedWithCorruptDatabaseIntercept(Interceptor::Chain& chain)
{
    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();
    return chain.proceed(chain.getRequest());
}

} /* namespace */

class CallWithDatabaseCorruptionIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

TEST_F(CallWithDatabaseCorruptionIntegrationTest,
        execute_DeletesCacheDatabaseAndAccessTheServer_WhenCacheDatabaseIsCorruptedBeforeCacheInitialization)
{
    // Given: store Url1 and Url2 in cache. Then, corrupt the cache database.
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl1;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl1, &handlerForUrl1);
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl2;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl2, &handlerForUrl2);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url1);
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url2);

    // create EasyHttp
    {
        HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
        EasyHttp::Builder httpClientBuilder;
        EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

        Request::Builder requestBuilder1;
        Request::Ptr pRequest1 = requestBuilder1.setUrl(Url1).build();
        Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
        Response::Ptr pResponse1 = pCall1->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
        pResponse1->getBody()->close();

        Request::Builder requestBuilder2;
        Request::Ptr pRequest2 = requestBuilder2.setUrl(Url2).build();
        Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
        Response::Ptr pResponse2 = pCall2->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
        pResponse2->getBody()->close();

        // Url1 and Url2 are in cache.
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        ASSERT_FALSE(db.getMetadataAll(key1).isNull());
        ASSERT_FALSE(db.getMetadataAll(key2).isNull());

        // free HttpCache, Easyhttp
    }

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    {
        Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
        EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
                WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

        // create new HttpCache and EasyHttp
        HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
        EasyHttp::Builder httpClientBuilder;
        EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).build();

        // When: execute with Url1
        Request::Builder requestBuilder1;
        Request::Ptr pRequest1 = requestBuilder1.setUrl(Url1).build();
        Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
        Response::Ptr pResponse1 = pCall1->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
        pResponse1->getBody()->close();

        // Then: don't use cache then access network (call network interceptor)
        // Url1 is in cache and Url2 is not in cache because recovered database.
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        ASSERT_FALSE(db.getMetadataAll(key1).isNull());
        ASSERT_TRUE(db.getMetadataAll(key2).isNull());

        // change Url1 handler to return bad request
        HttpTestCommonRequestHandler::BadRequestHandler badHandlerForUrl1;
        testServer.getTestRequestHandlerFactory().addHandler(PathForUrl1, &badHandlerForUrl1);

        // when request Url1 again cache is returned. (don't call network intercepter)
        // don't use bad request handler.
        Request::Builder requestBuilder3;
        Request::Ptr pRequest3 = requestBuilder3.setUrl(Url1).build();
        Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
        Response::Ptr pResponse3 = pCall3->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
        pResponse3->getBody()->close();
    }
}

// Windows では HttpCacheDatabase の database 初期化処理と Database のアクセスの間は database open 中のため
// database corruption にすることができないので )
TEST_F(CallWithDatabaseCorruptionIntegrationTest,
        execute_DeletesCacheDatabaseAndAccessTheServer_WhenCacheDatabaseIsCorruptedAfterCacheInitialization)
{
    // Given: store Url1 and Url2 in cache. Then, corrupt the cache database.
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl1;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl1, &handlerForUrl1);
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl2;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl2, &handlerForUrl2);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url1);
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url2);

    // create EasyHttp
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            Times(3).WillRepeatedly(testing::Invoke(delegateProceedOnlyIntercept));

    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).build();

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(Url1).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    pResponse1->getBody()->close();

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(Url2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    pResponse2->getBody()->close();

    {
        // Url1 and Url2 are in cache.
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        ASSERT_FALSE(db.getMetadataAll(key1).isNull());
        ASSERT_FALSE(db.getMetadataAll(key2).isNull());
    }

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    // When: execute with Url1
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(Url1).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    pResponse3->getBody()->close();

    // Then: don't use cache then access network (call network interceptor)
    // Url1 is in cache and Url2 is not in cache because recovered database.
    {
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        ASSERT_FALSE(db.getMetadataAll(key1).isNull());
        ASSERT_TRUE(db.getMetadataAll(key2).isNull());
    }
}

// Windows では database open 中のため Http Interceprtor で database corruption にすることができないので
// )
TEST_F(CallWithDatabaseCorruptionIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenDatabaseIsCorruptInGetReadableDatabaseOfGetMetadataOfHttpCacheDatabase)
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
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // close response body and put cache
    pResponse1->getBody()->close();

    // check cache
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key).isNull());

    // GET same url
    HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler2nd handler2nd;
    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2nd);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedWithCorruptDatabaseIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    Poco::Timestamp startTime2;

    // When: execute GET method. and receive NotModified and database is corrupt.
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pCall2->execute(), HttpExecutionException, 100702);

    // clear database
    EXPECT_EQ(0, pCache->getSize());
    EXPECT_TRUE(db.getMetadataAll(key).isNull());
}

} /* namespace test */
} /* namespace easyhttpcpp */
