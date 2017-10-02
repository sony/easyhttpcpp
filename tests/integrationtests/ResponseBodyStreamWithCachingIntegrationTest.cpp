/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
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

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"
#include "ResponseBodyStreamWithCaching.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "ResponseBodyStreamWithCachingIntegrationTest";

static const char* const HeaderTransferEncoding = "Transfer-Encoding";
static const char* const HeaderValueChunked = "chunked";

static const size_t ResponseBufferBytes = 8192;
static const int TestFailureTimeout = 10 * 1000; // milliseconds
static const int SkipTimoutSleepMilliSec = 500;

class ResponseBodyStreamWithCachingIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
};

namespace {

class ResponseTransferEncodingIsChunkedRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    ResponseTransferEncodingIsChunkedRequestHandler() : m_startSleep(false)
    {
    }

    virtual ~ResponseTransferEncodingIsChunkedRequestHandler()
    {
        if (m_startSleep) {
            m_finishEvent.tryWait(TestFailureTimeout);
        }
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.set(HeaderTransferEncoding, HeaderValueChunked);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
        ostr.flush();
        m_startSleep = true;
        Poco::Thread::sleep(SkipTimoutSleepMilliSec);
        ostr << "end";
    }
private:
    bool m_startSleep;
    Poco::Event m_finishEvent;
};

} /* namespace */

// Call::execute、Cache 使用で、ResponseBody を read
// temp file が作成される。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest, read_CreatesTemporaryFile_WhenAfterFirstRead)
{
    // Given: use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // not exist temporary directory
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    ASSERT_FALSE(tempDir.exists());

    // When: ResponesBodyStream::read
    ssize_t readSize = 10;
    EXPECT_GE(readSize, pResponseBodyStream->read(responseBodyBuffer.begin(), readSize));

    // Then: create temporary file
    Poco::Path tempPath(HttpTestUtil::getDefaultCacheTempDir());
    Poco::DirectoryIterator itDir(tempPath);
    Poco::DirectoryIterator itEnd;
    ASSERT_NE(itEnd, itDir) << "check to create temporary file.";
}

// temp ディレクトリの親ディレクトリをread only にして、Call::execute、Cache 使用で、ResponseBody を read
// exception はthrow されない。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        read_ReturnsReadSizeAndNotThrowsException_WhenIOErrorOccurredOnCreateTempDirectory)
{
    // Given: use cache and set read only to parent of temp directory
    Poco::File cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    cacheRootDir.createDirectories();
    cacheRootDir.setReadOnly(true);
    
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read
    // Then: not throw exception
    EXPECT_GE(strlen(HttpTestConstants::DefaultResponseBody), pResponseBodyStream->read(responseBodyBuffer.begin(),
            ResponseBufferBytes));
}

// temp ディレクトリをread only にして、Call::execute、Cache 使用で、ResponseBody を read
// exception はthrow されない。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        read_ReturnsReadSizeAndNotThrowsException_WhenIOErrorOccurredOnCreateTempFile)
{
    // Given: set read only to parent of temp directory
    Poco::File cacheTempDir(HttpTestUtil::getDefaultCacheRootDir());
    cacheTempDir.createDirectories();
    cacheTempDir.setReadOnly(true);
    
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read
    // Then: not throw exception
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);
    EXPECT_GE(strlen(HttpTestConstants::DefaultResponseBody), retSize);
}

// eof の後の、read
TEST_F(ResponseBodyStreamWithCachingIntegrationTest, read_ReturnsMinusOne_WhenAfterEof)
{
    // Given: use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // read response body until eof
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    ASSERT_TRUE(pResponseBodyStream->isEof());

    // When: ResponesBodyStream::read
    // Then: return -1
    EXPECT_EQ(-1, pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes));
}

// close を呼び出すと、Connection のreference count が decrement される。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        close_DecrementsConnectionReferenceCount_WhenHasConnection)
{
    // Given: use cache and start to read response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method
    Response::Ptr pResponse = pCall->execute();
    ASSERT_TRUE(pResponse->isSuccessful());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    ResponseBodyStreamWithCaching* pResponseBodyStreamWithCaching =
            static_cast<ResponseBodyStreamWithCaching*>(pResponseBodyStream.get());
    Connection::Ptr pConnection = pResponseBodyStreamWithCaching->getConnection();
    int beforeReferenceCount = pConnection->referenceCount();
    EASYHTTPCPP_LOG_D(Tag, "pConnection.referenceCount() = %d", beforeReferenceCount);

    // When: close response body stream with skip response body
    pResponseBodyStream->close();

    // Then: reference count was decremented 2 times. (ResponseBodyStream and HttpEngine)
    EXPECT_EQ(beforeReferenceCount - 2, pConnection->referenceCount());
}

// Call::execute、Cache 使用で、ResponseBody を全て read する前に、close、response body の skip で timeout。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest, close_NotStoresCache_WhenTheMiddleOfReadingAndSkipTimedOut)
{
    // Given: use cache and before read to the end of the response body stream.
    // set test handler
    HttpTestServer testServer;
    // contentLength=100, writeBytesBeforeSleep=20, sleepMilliSec=500
    HttpTestCommonRequestHandler::WaitInTheMiddleOfWriteResponseBodyRequestHandler handler(100, 20, 500);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // check response parameter
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // small size reading
    ssize_t readSize = 10;
    EXPECT_GE(readSize, pResponseBodyStream->read(responseBodyBuffer.begin(), readSize));

    // When:: close
    pResponseBodyStream->close();

    // Then: not store to cache
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_FALSE(db.getMetadataAll(key, metadata));
}

// Call::execute、Cache 使用で、Response の Content-Length 無の時、ResponseBody を全て readする前に、close
// response body の skip で timeout。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        close_NotStoreToCache_WhenContentLengthIsNotExistAndTheMiddleOfReadingAndSkipTimedOut)
{
    // Given: use cache and read no content-length response
    HttpTestServer testServer;
    ResponseTransferEncodingIsChunkedRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // small size reading
    ssize_t readSize = 10;
    EXPECT_GE(readSize, pResponseBodyStream->read(responseBodyBuffer.begin(), readSize));

    // When:: close
    pResponseBodyStream->close();

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

// Call::execute、Cache 使用で、Response の Content-Length 無の時、ResponseBody readする前に、close
// response body の skip で timeout。
TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        close_NotStoreToCache_WhenContentLengthIsNotExistAndSkipTimedOut)
{
    // Given: use cache and read no content-length response
    HttpTestServer testServer;
    ResponseTransferEncodingIsChunkedRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When:: close
    pResponseBodyStream->close();

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

TEST_F(ResponseBodyStreamWithCachingIntegrationTest,
        close_NotStoreToCache_WhenTempFileErrorOccuredInRead)
{
    // Given: set read only to parent of temp directory
    Poco::File cacheTempDir(HttpTestUtil::getDefaultCacheRootDir());
    cacheTempDir.createDirectories();
    cacheTempDir.setReadOnly(true);
    
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // ResponesBodyStream::read (temp file create fails.)
    size_t retSize = HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), retSize);

    // When:: close
    pResponseBodyStream->close();

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

} /* namespace test */
} /* namespace easyhttpcpp */
