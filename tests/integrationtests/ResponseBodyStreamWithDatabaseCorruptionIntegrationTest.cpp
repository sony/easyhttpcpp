/*
 * Copyright 2019 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/File.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/Connection.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "EasyHttpCppAssertions.h"
#include "HttpTestServer.h"
#include "TestLogger.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

namespace {

const char* const PathForUrl1 = "/path1";
const char* const PathForUrl2 = "/path2";
const char* const Url1 = "http://localhost:9982/path1";
const char* const Url2 = "http://localhost:9982/path2";

} /* namespace */

class ResponseBodyStreamWithDatabaseCorruptionIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    class DatabaseCorruptionInterceptor : public Interceptor {
    public:
        virtual Response::Ptr intercept(Chain& chain)
        {
            // corrupt cache database.
            HttpTestUtil::makeCacheDatabaseCorrupted();
            return chain.proceed(chain.getRequest());
        }
    };
};

// Windows では database open 中のため Http Interceprtor で database corruption にすることができないので
// )
TEST_F(ResponseBodyStreamWithDatabaseCorruptionIntegrationTest,
        close_DeletesCache_WhenCacheDatabaseIsCorruptedInGetWritableDatabaseOfUpdateMetadataOfHttpCacheDatabase)
{
    // Given: store Url1 in cache.
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl1;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl1, &handlerForUrl1);
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handlerForUrl2;
    testServer.getTestRequestHandlerFactory().addHandler(PathForUrl2, &handlerForUrl2);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url1);
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url2);

    // create cache
    {
        HttpCache::Ptr pCache1 = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
        EasyHttp::Builder httpClientBuilder1;
        EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache1).build();

        Request::Builder requestBuilder1;
        Request::Ptr pRequest1 = requestBuilder1.setUrl(Url1).build();
        Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);
        Response::Ptr pResponse1 = pCall1->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
        pResponse1->getBody()->close();

        // free HttpFileCache
    }

    {
        // corrupt database in interceptor
        Interceptor::Ptr pInterceptor = new DatabaseCorruptionInterceptor();

        HttpCache::Ptr pCache2 = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
        EasyHttp::Builder httpClientBuilder2;
        EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache2).addNetworkInterceptor(pInterceptor).build();

        Request::Builder requestBuilder2;
        Request::Ptr pRequest2 = requestBuilder2.setUrl(Url2).build();
        Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

        // execute url2
        Response::Ptr pResponse2 = pCall2->execute();
        EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

        // When: close response body
        pResponse2->getBody()->close();

        // Then: delete cache and response not store cache
        // delete database file
        Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
        Poco::File databaseFile(databasePath);
        EXPECT_FALSE(databaseFile.exists());

        // clear cache
        EXPECT_EQ(0, pCache2->getSize());
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        EXPECT_TRUE(db.getMetadataAll(key1).isNull());
        EXPECT_TRUE(db.getMetadataAll(key2).isNull());
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
