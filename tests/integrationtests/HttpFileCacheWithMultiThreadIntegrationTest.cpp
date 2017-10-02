/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Buffer.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/SharedPtr.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Headers.h"

#include "HttpIntegrationTestCase.h"
#include "HttpCacheMetadata.h"
#include "HttpFileCache.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"
#include "SynchronizedExecutionRunner.h"

using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "HttpFileCacheWithMultiThreadIntegrationTest";
static const char* const TestDataForCacheFromDb = "/HttpIntegrationTest/01_cache_from_db/HttpCache/cache";
static const char* const LruQuery1 = "test=1";

static const Request::HttpMethod Test1HttpMethod = Request::HttpMethodGet;
static const int Test1StatusCode = Poco::Net::HTTPResponse::HTTP_OK;
static const char* const Test1StatusMessage = "OK";
static const char* const Test1Header1 = "x-header1";
static const char* const Test1HeaderValue1 = "x-value1";
static const char* const Test1ResponseBody = "test1 response body";
static const char* const Test1SentRequestTime = "Fri, 05 Aug 2016 12:00:00 GMT";
static const char* const Test1ReceiveResponseTime = "Fri, 05 Aug 2016 12:00:10 GMT";
static const char* const Test1CreatedMetadataTime = "Fri, 05 Aug 2016 12:00:20 GMT";
static const char* const Test1TempFilename = "tempFile0001";

static const size_t ResponseBufferBytes = 8192;
static const int MultiThreadCount = 10;

namespace {

HttpCacheMetadata* createHttpCacheMetadata(const std::string& key, const std::string& url,
        size_t responseBodySize)
{
    HttpCacheMetadata* pHttpCacheMetadata = new HttpCacheMetadata();
    pHttpCacheMetadata->setKey(key);
    pHttpCacheMetadata->setUrl(url);
    pHttpCacheMetadata->setHttpMethod(Test1HttpMethod);
    pHttpCacheMetadata->setStatusCode(Test1StatusCode);
    pHttpCacheMetadata->setStatusMessage(Test1StatusMessage);
    Headers::Ptr pHeaders = new Headers();
    pHeaders->set(Test1Header1, Test1HeaderValue1);
    pHttpCacheMetadata->setResponseHeaders(pHeaders);
    pHttpCacheMetadata->setResponseBodySize(responseBodySize);
    Poco::Timestamp timeStamp;
    HttpUtil::tryParseDate(Test1SentRequestTime, timeStamp);
    pHttpCacheMetadata->setSentRequestAtEpoch(timeStamp.epochTime());
    HttpUtil::tryParseDate(Test1ReceiveResponseTime, timeStamp);
    pHttpCacheMetadata->setReceivedResponseAtEpoch(timeStamp.epochTime());
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, timeStamp);
    pHttpCacheMetadata->setCreatedAtEpoch(timeStamp.epochTime());
    return pHttpCacheMetadata;
}

std::string createResponseTempFile(int no)
{
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    tempDir.createDirectories();
    std::string tempFilename = StringUtil::format("%s%s_%d", HttpTestUtil::getDefaultCacheTempDir().c_str(),
            Test1TempFilename, no);
    Poco::FileOutputStream tempFileStream(tempFilename, std::ios::out | std::ios::trunc | std::ios::binary);
    tempFileStream.write(Test1ResponseBody, strlen(Test1ResponseBody));
    tempFileStream.close();
    return tempFilename;
}

class MethodExecutionRunner : public SynchronizedExecutionRunner {
public:
    MethodExecutionRunner(HttpFileCache* pHttpFileCache, const std::string& key, const std::string& url) :
            m_pHttpFileCache(pHttpFileCache), m_key(key), m_url(url)
    {
    }
    const std::string& getKey()
    {
        return m_key;
    }
    const std::string& getUrl()
    {
        return m_url;
    }
protected:
    HttpFileCache* m_pHttpFileCache;
    const std::string m_key;
    const std::string m_url;
};

class GetMetadataExecutionRunner : public MethodExecutionRunner {
public:
    GetMetadataExecutionRunner(HttpFileCache* pHttpFileCache, const std::string& key, const std::string& url) :
            MethodExecutionRunner(pHttpFileCache, key, url)
    {
    }
    bool execute()
    {
        // prepare
        m_pCacheMetadata = new HttpCacheMetadata;

        setToReady();
        waitToStart();

        // execute
        return m_pHttpFileCache->getMetadata(m_key, m_pCacheMetadata);
    }
    CacheMetadata::Ptr getCacheMetadata()
    {
        return m_pCacheMetadata;
    }
private:
    CacheMetadata::Ptr m_pCacheMetadata;
};

class GetDataExecutionRunner : public MethodExecutionRunner {
public:
    GetDataExecutionRunner(HttpFileCache* pHttpFileCache, const std::string& key, const std::string& url) :
            MethodExecutionRunner(pHttpFileCache, key, url)
    {
    }
    bool execute()
    {
        // prepare
        std::istream* pStream = NULL;

        setToReady();
        waitToStart();

        // execute
        bool ret = m_pHttpFileCache->getData(m_key, pStream);
        m_pStreamPtr = pStream;
        return ret;
    }
    std::istream* getStream()
    {
        return m_pStreamPtr;
    }
private:
    Poco::SharedPtr<std::istream> m_pStreamPtr;
};

class PutExecutionRunner : public MethodExecutionRunner {
public:
    PutExecutionRunner(HttpFileCache* pHttpFileCache, const std::string& key, const std::string& url,
            const std::string& tempFilePath, size_t responseBodySize) :
            MethodExecutionRunner(pHttpFileCache, key, url),
            m_tempFilePath(tempFilePath), m_responseBodySize(responseBodySize)
    {
    }
    bool execute()
    {
        // prepare
        CacheMetadata::Ptr pCacheMetadata = createHttpCacheMetadata(m_key, m_url, m_responseBodySize);

        setToReady();
        waitToStart();

        // execute
        return m_pHttpFileCache->put(m_key, pCacheMetadata, m_tempFilePath);
    }
private:
    const std::string m_tempFilePath;
    const size_t m_responseBodySize;
};

class RemoveExecutionRunner : public MethodExecutionRunner {
public:
    RemoveExecutionRunner(HttpFileCache* pHttpFileCache, const std::string& key, const std::string& url) :
            MethodExecutionRunner(pHttpFileCache, key, url)
    {
    }
    bool execute()
    {
        // prepare

        setToReady();
        waitToStart();

        // execute
        return m_pHttpFileCache->remove(m_key);
    }
};

class PurgeExecutionRunner : public SynchronizedExecutionRunner {
public:
    PurgeExecutionRunner(HttpFileCache* pHttpFileCache) : m_pHttpFileCache(pHttpFileCache)
    {
    }
    bool execute()
    {
        // prepare
        setToReady();
        waitToStart();

        // execute
        m_startTime.update();
        bool ret = m_pHttpFileCache->purge(true);
        m_endTime.update();
        return ret;
    }
    Poco::Timestamp getStartTime()
    {
        return m_startTime;
    }
    Poco::Timestamp getEndTime()
    {
        return m_endTime;
    }
private:
    HttpFileCache* m_pHttpFileCache;
    Poco::Timestamp m_startTime;
    Poco::Timestamp m_endTime;
};

bool prepareToCreateCache(HttpFileCache* pHttpFileCache)
{
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        CacheMetadata::Ptr pCacheMetadata = createHttpCacheMetadata(key, url, strlen(Test1ResponseBody));
        if (!pHttpFileCache->put(key, pCacheMetadata, tempFilePath)) {
            EASYHTTPCPP_LOG_D(Tag, "prepareToCreateCache: failed. [%d] url=%s", i, url.c_str());
            return false;
        }
    }
    return true;
}

}   /* namespace */

class HttpFileCacheWithMultiThreadIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
    Poco::AutoPtr<MethodExecutionRunner> m_pExecutionRunners[MultiThreadCount];
};

// getMetadata を multi thread から同時呼び出し
// それぞれ別のCache に存在する url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        getMetadata_ReturnsTrueAndGetMetadata_WhenCalledOnMultiThreadAndExistInCache)
{
    // Given: create each cache and create thread and execute before getMetadata
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // create cache
    ASSERT_TRUE(prepareToCreateCache(&httpFileCache));

    // execute before getMetadata
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new GetMetadataExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start getMetadata
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: getMetadata succeeded
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(HttpTestUtil::getDefaultCachePath()));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        HttpCacheMetadata* pHttpCacheMetadata =
                static_cast<HttpCacheMetadata*>(
                static_cast<GetMetadataExecutionRunner*>(m_pExecutionRunners[i].get())->getCacheMetadata().get());
        EXPECT_TRUE(db.getMetadataAll(m_pExecutionRunners[i]->getKey(), metadata));
        EXPECT_EQ(metadata.getKey(), pHttpCacheMetadata->getKey());
        EXPECT_EQ(metadata.getUrl(), pHttpCacheMetadata->getUrl());
        EXPECT_EQ(metadata.getHttpMethod(), pHttpCacheMetadata->getHttpMethod());
    }
}

// getMetadata を multi thread から同時呼び出し
//同じ url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        getMetadata_ReturnsTrueAndGetMetadata_WhenCalledOnMultiThreadAndExistInCacheAndSameUrl)
{
    // Given: create cache and create thread and execute before getMetadata
    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb);
    srcTestData.copyTo(cachePath);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        m_pExecutionRunners[i] = new GetMetadataExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start getMetadata
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: getMetadata succeeded
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        HttpCacheMetadata* pHttpCacheMetadata =
                static_cast<HttpCacheMetadata*>(
                static_cast<GetMetadataExecutionRunner*>(m_pExecutionRunners[i].get())->getCacheMetadata().get());
        EXPECT_TRUE(db.getMetadataAll(m_pExecutionRunners[i]->getKey(), metadata));
        EXPECT_EQ(metadata.getKey(), pHttpCacheMetadata->getKey());
        EXPECT_EQ(metadata.getUrl(), pHttpCacheMetadata->getUrl());
        EXPECT_EQ(metadata.getHttpMethod(), pHttpCacheMetadata->getHttpMethod());
    }
}

// getData を multi thread から同時呼び出し
// それぞれ別 url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        getData_ReturnsTrueAndGetData_WhenCalledOnMultiThreadAndExistInCache)
{
    // Given: create each cache and create thread and execute before getData
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // create cache
    ASSERT_TRUE(prepareToCreateCache(&httpFileCache));

    // execute before getData
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new GetDataExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start getData
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: getData succeeded
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        std::istream* pStream = static_cast<GetDataExecutionRunner*>(m_pExecutionRunners[i].get())->getStream();
        Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
        pStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);
        size_t readBytes = pStream->gcount();
        
        Poco::File cachedBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(HttpTestUtil::getDefaultCachePath(),
                Request::HttpMethodGet, m_pExecutionRunners[i]->getUrl()));
        EXPECT_EQ(readBytes, cachedBodyFile.getSize());
        Poco::FileInputStream cachedBodyStream(cachedBodyFile.path(), std::ios::in | std::ios::binary);
        Poco::Buffer<char> inBuffer(readBytes);
        cachedBodyStream.read(inBuffer.begin(), readBytes);
        EXPECT_EQ(readBytes, cachedBodyStream.gcount());
        EXPECT_EQ(0, memcmp(inBuffer.begin(), responseBodyBuffer.begin(), readBytes));
    }
}

// getData を multi thread から同時呼び出し
// 同じ url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        getData_ReturnsTrueAndGetData_WhenCalledOnMultiThreadAndExistInCacheAndSameUrl)
{
    // Given: create cache and create thread and execute before getData
    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb);
    srcTestData.copyTo(cachePath);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        m_pExecutionRunners[i] = new GetDataExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start getData
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: getData succeeded
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        std::istream* pStream = static_cast<GetDataExecutionRunner*>(m_pExecutionRunners[i].get())->getStream();
        Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
        pStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);
        size_t readBytes = pStream->gcount();
        
        Poco::File cachedBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
                m_pExecutionRunners[i]->getUrl()));
        EXPECT_EQ(readBytes, cachedBodyFile.getSize());
        Poco::FileInputStream cachedBodyStream(cachedBodyFile.path(), std::ios::in | std::ios::binary);
        Poco::Buffer<char> inBuffer(readBytes);
        cachedBodyStream.read(inBuffer.begin(), readBytes);
        EXPECT_EQ(readBytes, cachedBodyStream.gcount());
        EXPECT_EQ(0, memcmp(inBuffer.begin(), responseBodyBuffer.begin(), readBytes));
    }
}

// put を multi thread から同時呼び出し
// それぞれ別のCache に存在しない url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        put_ReturnsTrueAndPutData_WhenCalledOnMultiThreadAndNotExistInCache)
{
    // Given: create thread and execute before put
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new PutExecutionRunner(&httpFileCache, key, url, tempFilePath,
                strlen(Test1ResponseBody));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start put
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: put succeeded
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        EXPECT_TRUE(db.getMetadataAll(m_pExecutionRunners[i]->getKey(), metadata));
        Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
                m_pExecutionRunners[i]->getUrl()));
        EXPECT_TRUE(responseBodyFile.exists());
    }
}

// put を multi thread から同時呼び出し
// それぞれ別のCache に存在する url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        put_ReturnsTrueAndUpdateData_WhenCalledOnMultiThreadAndExistInCache)
{
    // Given: create each cache and create thread and execute before put
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // create cache
    ASSERT_TRUE(prepareToCreateCache(&httpFileCache));

    // execute before put
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new PutExecutionRunner(&httpFileCache, key, url, tempFilePath,
                strlen(Test1ResponseBody));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start put
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: put succeeded
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        EXPECT_TRUE(db.getMetadataAll(m_pExecutionRunners[i]->getKey(), metadata));
        Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
                m_pExecutionRunners[i]->getUrl()));
        EXPECT_TRUE(responseBodyFile.exists());
    }
}

// put を multi thread から同時呼び出し
// 同じurl
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        put_ReturnsTrueAndPuLastAccessedData_WhenCalledOnMultiThreadAndExistInCacheAndSameUrl)
{
    // Given: create cache and create thread and execute before put
    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb);
    srcTestData.copyTo(cachePath);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new PutExecutionRunner(&httpFileCache, key, url, tempFilePath,
                strlen(Test1ResponseBody));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start put
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: put succeeded
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
    }

    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    EXPECT_TRUE(db.getMetadataAll(m_pExecutionRunners[0]->getKey(), metadata));
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            m_pExecutionRunners[0]->getUrl()));
    EXPECT_TRUE(responseBodyFile.exists());
}

// remove を multi thread から同時呼び出し
// それぞれ別 url
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        remove_ReturnsTrueAndRemoveMetadataAndData_WhenCalledOnMultiThreadAndExistInCache)
{
    // Given: create each cache and create thread and execute before remove
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // create cache
    ASSERT_TRUE(prepareToCreateCache(&httpFileCache));

    // execute before remove
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new RemoveExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start remove
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: remove succeeded
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutionRunners[i]->isSuccess());
        EXPECT_FALSE(db.getMetadataAll(m_pExecutionRunners[i]->getKey(), metadata));
        Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
                m_pExecutionRunners[i]->getUrl()));
        EXPECT_FALSE(responseBodyFile.exists());
    }
}

// removeを multi thread から同時呼び出し
// 同じurl
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        remove_ReturnsTrueAtFirstExecutionAndReturnsFalseAnotherExecution_WhenCalledOnMultiThreadAndExistInCacheAndSameUrl)
{
    // Given: create cache and create thread and execute before remove
    // test data LRU (Query) old -> new
    // LRU_QUERY3
    // LRU_QUERY1
    // LRU_QUERY4
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    Poco::File cacheParentPath(HttpTestUtil::getDefaultCacheParentPath());
    cacheParentPath.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForCacheFromDb);
    srcTestData.copyTo(cachePath);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, LruQuery1);
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        std::string tempFilePath = createResponseTempFile(i);
        m_pExecutionRunners[i] = new RemoveExecutionRunner(&httpFileCache, key, url);
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutionRunners[i]);
        m_pExecutionRunners[i]->waitToReady();
    }

    // When: start remove
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutionRunners[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: remove succeeded 1 times
    int successCount = 0;
    for (int i = 0; i < MultiThreadCount; i++) {
        if (m_pExecutionRunners[i]->isSuccess()) {
            successCount++;
        }
    }
    EXPECT_EQ(1, successCount);

    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    EXPECT_FALSE(db.getMetadataAll(m_pExecutionRunners[0]->getKey(), metadata));
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath, Request::HttpMethodGet,
            m_pExecutionRunners[0]->getUrl()));
    EXPECT_FALSE(responseBodyFile.exists());
}

// purge 中に、別のスレッドからgetMetadata する。
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        getMetadata_ReturnsFalseAfterPurge_WhenDuringExecutionOfPurgeInOtherThread)
{
    // Given: create some metadata for purge
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // 50ms の sleep から、50ms ずつふやして、purge 中に method 呼び出しが行なわれるまで、
    // テストを繰り返します。
    for (long sleepMilliSec = 50; sleepMilliSec < 200; sleepMilliSec += 50) {

        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        std::string lastKey;
        for (int i = 0; i < 20; i++) {
            std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                    HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
            std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
            std::string tempFilePath = createResponseTempFile(i);
            CacheMetadata::Ptr pCacheMetadata = createHttpCacheMetadata(key, url, strlen(Test1ResponseBody));
            ASSERT_TRUE(httpFileCache.put(key, pCacheMetadata, tempFilePath));
            lastKey = key;
        }

        PurgeExecutionRunner runner(&httpFileCache);
        Poco::ThreadPool threadPool;
        threadPool.start(runner);
        runner.waitToReady();
        runner.setToStart();

        CacheMetadata::Ptr pCacheMetadata = new HttpCacheMetadata;
        Poco::Thread::sleep(sleepMilliSec);    // millisecond

        // When : getMetadata
        // Then : return false
        Poco::Timestamp methodStartTime;
        EXPECT_FALSE(httpFileCache.getMetadata(lastKey, pCacheMetadata));
        Poco::Timestamp methodEndTime;

        threadPool.joinAll();

        // purge, method の開始、終了時間を表示します。
        Poco::Timestamp purgeStartTime = runner.getStartTime();
        Poco::Timestamp purgeEndTime = runner.getEndTime();
        long long purgeStartMicroSec = static_cast<long long>(purgeStartTime.epochMicroseconds());
        long long purgeEndMicroSec = static_cast<long long>(purgeEndTime.epochMicroseconds());
        long long methodStartMicroSec = static_cast<long long>(methodStartTime.epochMicroseconds());
        long long methodEndMicroSec = static_cast<long long>(methodEndTime.epochMicroseconds());
        EASYHTTPCPP_LOG_I(Tag, "sleepMiiliSec=%ld", sleepMilliSec);
        EASYHTTPCPP_LOG_I(Tag, "purge start=%lld end=%lld elapsed=%lld", purgeStartMicroSec, purgeEndMicroSec,
                purgeEndMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "getMetadata start=%lld end=%lld elapsed=%lld from purge=%lld",
                methodStartMicroSec, methodEndMicroSec, methodEndMicroSec - methodStartMicroSec,
                methodStartMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "purge start=%lld", purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "start      =%lld (diff=%lld)", methodStartMicroSec,
                methodStartMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "purge end  =%lld (diff=%lld)", purgeEndMicroSec, purgeEndMicroSec - methodStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "end        =%lld (diff=%lld)", methodEndMicroSec, methodEndMicroSec - purgeEndMicroSec);

        // method の開始時間が、purge より早い場合は、sleep 時間をふやして retry する。
        if (methodStartMicroSec <= purgeStartMicroSec) {
            continue;
        }

        // check completion of purge
        std::string cachePath = HttpTestUtil::getDefaultCachePath();
        HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
        HttpCacheDatabase::HttpCacheMetadataAll metadata;
        for (int i = 0; i < 20; i++) {
            std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                    HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
            std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
            
            EXPECT_FALSE(db.getMetadataAll(key, metadata));
            Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
                    Request::HttpMethodGet, url));
            EXPECT_FALSE(responseBodyFile.exists());
        }
        // finish test
        return;
    }
}

// purge 中に、別のスレッドからput する。
TEST_F(HttpFileCacheWithMultiThreadIntegrationTest,
        put_ReturnsTrueAfterPurge_WhenDuringExecutionOfPurgeInOtherThread)
{
    // Given: create some metadata for purge
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache httpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    // 50ms の sleep から、50ms ずつふやして、purge 中に method 呼び出しが行なわれるまで、
    // テストを繰り返します。
    for (long sleepMilliSec = 50; sleepMilliSec < 200; sleepMilliSec += 50) {

        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        for (int i = 0; i < 20; i++) {
            std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                    HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
            std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
            std::string tempFilePath = createResponseTempFile(i);
            CacheMetadata::Ptr pCacheMetadata = createHttpCacheMetadata(key, url, strlen(Test1ResponseBody));
            ASSERT_TRUE(httpFileCache.put(key, pCacheMetadata, tempFilePath));
        }

        PurgeExecutionRunner runner(&httpFileCache);
        Poco::ThreadPool threadPool;
        threadPool.start(runner);
        runner.waitToReady();
        runner.setToStart();

        CacheMetadata::Ptr pCacheMetadata = new HttpCacheMetadata;
        Poco::Thread::sleep(sleepMilliSec);    // millisecond

        int targetId = 30;
        std::string targetUrl = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", targetId));
        std::string targetKey = HttpUtil::makeCacheKey(Request::HttpMethodGet, targetUrl);
        std::string targetTempFilePath = createResponseTempFile(targetId);
        CacheMetadata::Ptr pTargetCacheMetadata = createHttpCacheMetadata(targetKey, targetUrl,
                strlen(Test1ResponseBody));

        // When : put
        // Then : return true
        Poco::Timestamp methodStartTime;
        EXPECT_TRUE(httpFileCache.put(targetKey, pTargetCacheMetadata, targetTempFilePath));
        Poco::Timestamp methodEndTime;

        threadPool.joinAll();

        // purge, method の開始、終了時間を表示します。
        Poco::Timestamp purgeStartTime = runner.getStartTime();
        Poco::Timestamp purgeEndTime = runner.getEndTime();
        long long purgeStartMicroSec = static_cast<long long>(purgeStartTime.epochMicroseconds());
        long long purgeEndMicroSec = static_cast<long long>(purgeEndTime.epochMicroseconds());
        long long methodStartMicroSec = static_cast<long long>(methodStartTime.epochMicroseconds());
        long long methodEndMicroSec = static_cast<long long>(methodEndTime.epochMicroseconds());
        EASYHTTPCPP_LOG_I(Tag, "sleepMiiliSec=%ld", sleepMilliSec);
        EASYHTTPCPP_LOG_I(Tag, "purge start=%lld end=%lld elapsed=%lld", purgeStartMicroSec, purgeEndMicroSec,
                purgeEndMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "getMetadata start=%lld end=%lld elapsed=%lld from purge=%lld",
                methodStartMicroSec, methodEndMicroSec, methodEndMicroSec - methodStartMicroSec,
                methodStartMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "purge start=%lld", purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "start      =%lld (diff=%lld)", methodStartMicroSec,
                methodStartMicroSec - purgeStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "purge end  =%lld (diff=%lld)", purgeEndMicroSec, purgeEndMicroSec - methodStartMicroSec);
        EASYHTTPCPP_LOG_I(Tag, "end        =%lld (diff=%lld)", methodEndMicroSec, methodEndMicroSec - purgeEndMicroSec);

        // method の開始時間が、purge より早い場合は、sleep 時間をふやして retry する。
        if (methodStartMicroSec <= purgeStartMicroSec) {
            continue;
        }

        // check completion of purge
        std::string cachePath = HttpTestUtil::getDefaultCachePath();
        HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
        HttpCacheDatabase::HttpCacheMetadataAll metadata;
        for (int i = 0; i < 20; i++) {
            std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                    HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, StringUtil::format("no=%d", i));
            std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
            
            EXPECT_FALSE(db.getMetadataAll(key, metadata));
            Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
                    Request::HttpMethodGet, url));
            EXPECT_FALSE(responseBodyFile.exists());
        }

        // check put
        EXPECT_TRUE(db.getMetadataAll(targetKey, metadata));
        Poco::File targetResponseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
                Request::HttpMethodGet, targetUrl));
        EXPECT_TRUE(targetResponseBodyFile.exists());

        // finish test
        return;
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
