/*
 * Copyright 2019 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "HttpTestServer.h"
#include "EasyHttpCppAssertions.h"
#include "TestFileUtil.h"
#include "TestLogger.h"

#include "HttpIntegrationTestCase.h"
#include "HttpFileCache.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::TestFileUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

namespace {

const char* const LruQuery1 = "test=1";
const char* const LruQuery2 = "test=2";
const char* const LruQuery3 = "test=3";
const char* Test1Url = "http://localhost:9982/test1?a=10";
const char* Test2Url = "http://localhost:9982/test2?a=10";
const char* const Test1ResponseBody = "test1 response body";
const char* const Test2ResponseBody = "test2 response body";
const char* const Test1TempFilename = "tempFile0001";

size_t getFileCount(Poco::File& dir)
{
    std::vector<Poco::File> fileLists;
    dir.list(fileLists);
    size_t fileCount = 0;
    for (size_t i = 0; i < fileLists.size(); i++) {
        if (fileLists[i].isFile()) {
            fileCount++;
        }
    }
    return fileCount;
}

} /* namespace */

class HttpFileCacheDatabaseCorruptIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        TestFileUtil::setFullAccess(path);
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest, getSize_ThrowsHttpExecutionException_WhenCacheDatabaseIsCorrupt)
{
    // Given: database is corrupt
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // database is corrupt.
    HttpTestUtil::createInvalidCacheDatabase();

    // When: getSize
    // Then: throw HttpExecutionException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(httpFileCache.getSize(), HttpExecutionException, 100702);
}

TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest,
        getData_ReturnsFalseAndDeletesCache_WhenDatabaseIsCorruptInGetReadableDatabaseOfEnumerateOfHttpCacheDatabase)
{
    // Given: prepare cache database and free HttpFileCache
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    {
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);
        HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
                strlen(Test1ResponseBody));
        std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
        ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));
    }

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    {
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

        // When: getData
        // Then: return false
        std::istream* pStream = NULL;
        EXPECT_FALSE(pHttpFileCache->getData(key, pStream));

        // delete database file
        Poco::File databaseFile(databasePath);
        EXPECT_FALSE(databaseFile.exists());

        // delete all cache response body file
        Poco::File cacheRoot(HttpTestUtil::getDefaultCacheRootDir());
        EXPECT_EQ(0, getFileCount(cacheRoot));

        // clear cache
        EXPECT_EQ(0, pHttpFileCache.unsafeCast<HttpFileCache>()->getSize());
    }
}

TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest,
        get_ReturnsFalseAndDeletesCache_WhenDatabaseIsCorruptInGetReadableDatabaseOfEnumerateOfHttpCacheDatabase)
{
    // Given: prepare cache database and free HttpFileCache
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    {
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);
        HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
                strlen(Test1ResponseBody));
        std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
        ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));
    }

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    {
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

        // When: get
        // Then: return false
        std::istream* pStream = NULL;
        CacheMetadata::Ptr pHttpCacheMetadata;
        EXPECT_FALSE(pHttpFileCache->get(key, pHttpCacheMetadata, pStream));

        // delete database file
        Poco::File databaseFile(databasePath);
        EXPECT_FALSE(databaseFile.exists());

        // delete all cache response body file
        Poco::File cacheRoot(HttpTestUtil::getDefaultCacheRootDir());
        EXPECT_EQ(0, getFileCount(cacheRoot));

        // clear cache
        EXPECT_EQ(0, pHttpFileCache.unsafeCast<HttpFileCache>()->getSize());
    }
}

TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest,
        putMetadata_ReturnsFalseAndDeletesCache_WhenDatabaseIsCorruptInGetWritableDatabaseOfEnumerateOfHttpCacheDatabase)
{
    // Given: prepare cache database and free HttpFileCache
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    {
        std::string url = Test1Url;
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);
        HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
                strlen(Test1ResponseBody));
        std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
        ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));
    }

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    {
        HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);
        std::string url = Test2Url;
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
                strlen(Test2ResponseBody));

        // When: putMetadata
        // Then: return false
        EXPECT_FALSE(pHttpFileCache->putMetadata(key, pHttpCacheMetadata));

        // delete database file
        Poco::File databaseFile(databasePath);
        EXPECT_FALSE(databaseFile.exists());

        // delete all cache response body file
        Poco::File cacheRoot(HttpTestUtil::getDefaultCacheRootDir());
        EXPECT_EQ(0, getFileCount(cacheRoot));

        // clear cache
        EXPECT_EQ(0, pHttpFileCache.unsafeCast<HttpFileCache>()->getSize());
    }
}

// Windwos では HttpFileCache::remove と HttpFileCache::releaseData の database open 中に database corruption
// にすることができないので )
TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest,
        releseData_ReturnsTrueAndDeletesCache_WhenDatabaseIsCorruptInGetWritableDatabaseOfDeleteMetadataOfHttpCacheDatabase)
{
    // Given: prepare cache database and turn on remove flag.
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    std::string url1 = Test1Url;
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    HttpCacheMetadata::Ptr pHttpCacheMetadata1 = HttpTestUtil::createHttpCacheMetadata(key1, url1,
            strlen(Test1ResponseBody));
    std::string tempFilePath1 = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody, 1);
    ASSERT_TRUE(pHttpFileCache->put(key1, pHttpCacheMetadata1, tempFilePath1));

    std::string url2 = Test2Url;
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    HttpCacheMetadata::Ptr pHttpCacheMetadata2 = HttpTestUtil::createHttpCacheMetadata(key2, url2,
            strlen(Test1ResponseBody));
    std::string tempFilePath2 = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody, 2);
    ASSERT_TRUE(pHttpFileCache->put(key2, pHttpCacheMetadata2, tempFilePath2));

    // turn on remove flag by get and remove
    CacheMetadata::Ptr pCacheMetadata3 = new HttpCacheMetadata();
    std::istream* pStream = NULL;
    ASSERT_TRUE(pHttpFileCache->get(key1, pCacheMetadata3, pStream));
    delete pStream;
    ASSERT_TRUE(pHttpFileCache->remove(key1));

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    // When: releaseData
    // Then: delete cache database
    pHttpFileCache->releaseData(key1);

    // delete database file
    Poco::File databaseFile(databasePath);
    EXPECT_FALSE(databaseFile.exists());

    // delete all cache response body file
    Poco::File cacheRoot(HttpTestUtil::getDefaultCacheRootDir());
    EXPECT_EQ(0, getFileCount(cacheRoot));

    // clear cache
    EXPECT_EQ(0, pHttpFileCache.unsafeCast<HttpFileCache>()->getSize());
}

// Windows では Database open 中に database corruption にすることができないので )
TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest,
        purge_ReturnsTrueAndDeletesCache_WhenDatabaseIsCorruptInGetWritableDatabaseOfDeleteMetadataOfHttpCacheDatabase)
{
    // Given: prepare cache database
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    std::string url1 = Test1Url;
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    HttpCacheMetadata::Ptr pHttpCacheMetadata1 = HttpTestUtil::createHttpCacheMetadata(key1, url1,
            strlen(Test1ResponseBody));
    std::string tempFilePath1 = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody, 1);
    ASSERT_TRUE(pHttpFileCache->put(key1, pHttpCacheMetadata1, tempFilePath1));

    // corrupt cache database.
    HttpTestUtil::makeCacheDatabaseCorrupted();

    // When: purge
    // Then: return true and delete cache database
    EXPECT_TRUE(pHttpFileCache->purge(true));

    // delete database file
    Poco::File databaseFile(databasePath);
    EXPECT_FALSE(databaseFile.exists());

    // delete all cache response body file
    Poco::File cacheRoot(HttpTestUtil::getDefaultCacheRootDir());
    EXPECT_EQ(0, getFileCount(cacheRoot));

    // clear cache
    EXPECT_EQ(0, pHttpFileCache.unsafeCast<HttpFileCache>()->getSize());
}

// database corrupt の時の evictAll
TEST_F(HttpFileCacheDatabaseCorruptIntegrationTest, evictAll_DeletesAllCache_WhenCacheDatabaseIsCorrupt)
{
    // Given: create cache in some url then cache database is corrupt.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    std::string url1 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
        HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
        HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery2);
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);

    {
        HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

        // create EasyHttp
        EasyHttp::Builder httpClientBuilder;
        EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

        // request 1
        Request::Builder requestBuilder1;
        Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
        Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
        Response::Ptr pResponse1 = pCall1->execute();
        // read response body and close
        std::string responseBody1 = pResponse1->getBody()->toString();

        // request 2
        Request::Builder requestBuilder2;
        Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
        Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
        Response::Ptr pResponse2 = pCall2->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
        // read response body and close
        std::string responseBody2 = pResponse2->getBody()->toString();

        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url1);
        ASSERT_FALSE(db.getMetadataAll(key1).isNull());
        std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
        ASSERT_FALSE(db.getMetadataAll(key2).isNull());

        Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
        ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile1.getSize());
        Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
        ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile2.getSize());

        ASSERT_TRUE(tempDir.exists());
    }

    // corrupt cache database.
    // Windows では Database open 中に database corruption することができないので、
    // EasyHttp, HttpCache を解放してから database corruption します。
    HttpTestUtil::makeCacheDatabaseCorrupted();

    {
        HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

        // When: call evictAll
        pCache->evictAll();

        // Then: remove all cache
        HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
        EXPECT_TRUE(db.getMetadataAll(key1).isNull());
        EXPECT_TRUE(db.getMetadataAll(key2).isNull());

        Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url1));
        EXPECT_FALSE(responseBodyFile1.exists());
        Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url2));
        EXPECT_FALSE(responseBodyFile2.exists());

        EXPECT_FALSE(tempDir.exists());

        // create EasyHttp
        EasyHttp::Builder httpClientBuilder;
        EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
 
        // cache が使えるか確認
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

        std::string key3 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url3);
        EXPECT_FALSE(db.getMetadataAll(key3).isNull());

        Poco::File responseBodyFile3(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            url3));
        EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), responseBodyFile3.getSize());
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
