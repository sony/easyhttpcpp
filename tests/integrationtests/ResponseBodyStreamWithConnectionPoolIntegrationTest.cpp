/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Interceptor.h"
#include "HeaderContainMatcher.h"
#include "HttpTestServer.h"
#include "TestLogger.h"

#include "CallInternal.h"
#include "ConnectionConfirmationInterceptor.h"
#include "ConnectionInternal.h"
#include "HttpEngine.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "EasyHttpInternal.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const char* const HeaderConnection = "Connection";
static const char* const HeaderValueKeepAlive = "Keep-Alive";
static const char* const HeaderValueClose = "Close";
static const size_t ResponseBufferBytes = 8192;

class ResponseBodyStreamWithConnectionPoolIntegrationTest : public HttpIntegrationTestCase {
protected:

   void SetUp()
    {
        Poco::Path cachePath(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(cachePath);

        Poco::Path certRootDir(HttpTestUtil::getDefaultCertRootDir());
        FileUtil::removeDirsIfPresent(certRootDir);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

// keep-alive timeout でConnection 削除
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest, close_StartsKeepAliveTimer_WhenConnectionKeepAlive)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: add Connection to ConnectionPool.
    unsigned int keepAliveTimeoutSec = 2;   // 2 sec
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool(10, keepAliveTimeoutSec);
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();

    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute communication.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // When: read and close
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    ASSERT_EQ(1, pConnectionPool->getTotalConnectionCount());
    ASSERT_EQ(1, pConnectionPool->getKeepAliveIdleConnectionCount());

    // wait Keep-Alive timeout time
    Poco::Thread::sleep(static_cast<long>(keepAliveTimeoutSec) * 1000L + 500L); // margin 500ms

    // Then: remove Connection from ConnectionPool.
    ASSERT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

// HttpEngine が解放されても Connection は解放されないことの確認 (Connection:Keep-Alive)
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest,
        close_AddsConnectionToConnectionPool_WhenReleaseCallInstanceBeforeCloseResponseBodyStream)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool
    // release Call Instance.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor(pConfirmationInterceptor);
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_THAT(pResponse->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    Connection::Ptr pConnection = pConfirmationInterceptor->getConnection();
    ConnectionInternal* pConnectionInternalPtr = pConnection.unsafeCast<ConnectionInternal>();
    pConfirmationInterceptor->clearConnection();

    HttpEngine::Ptr pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();

    // check reference count
    ASSERT_EQ(4, pConnection->referenceCount());    // from this function and ResponseBodyStream and HttpEngine and
                                                    // ConnectionPoolInternal
    ASSERT_EQ(2, pHttpEngine->referenceCount());    // from this function and CallInternal
    ASSERT_EQ(1, pCall->referenceCount());
    pConnection = NULL;
    pHttpEngine = NULL;
    pCall = NULL;

    // When: read end close response body stream
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    // Then: Connection is added to ConnectionPool with Idle.
    EXPECT_EQ(1, pConnectionPool->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPool->getKeepAliveIdleConnectionCount());
    EXPECT_TRUE(static_cast<ConnectionPoolInternal*>(pConnectionPool.get())->isConnectionExisting(pConnectionInternalPtr));
}

// HttpEngine が解放されても Connection は解放されないことの確認 (Connection:Close)
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest,
        close_NotAddConnectionToConnectionPool_WhenReleaseCallInstanceBeforeCloseResponseBodyStreamWithConnectionClose)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::ConnectionCloseRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool
    // release Call Instance.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor(pConfirmationInterceptor);
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_THAT(pResponse->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueClose));

    Connection::Ptr pConnection = pConfirmationInterceptor->getConnection();
    pConfirmationInterceptor->clearConnection();

    HttpEngine::Ptr pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();

    ASSERT_EQ(3, pConnection->referenceCount());    // from this function and ResponseBodyStream and HttpEngine
    ASSERT_EQ(2, pHttpEngine->referenceCount());    // from this function and CallInternal
    ASSERT_EQ(1, pCall->referenceCount());
    pConnection = NULL;
    pHttpEngine = NULL;
    pCall = NULL;

    // When: read and close response body stream
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    // Then: Connection is nod added to ConnectionPool.
    EXPECT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

// max keep-alive idle count オーバー
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest,
        close_RemovesOldestKeepAliveTimeoutConnection_WhenExceedsMaxKeepAliveIdleCount)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool.
    // add Connection until max Keep-Alive idle count (3).
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool(3, 60);
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor(pConfirmationInterceptor);
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    // 1st Connection
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    ConnectionInternal* pConnectionInternalPtr1 = pConnection1.unsafeCast<ConnectionInternal>();
    pConnection1 = NULL;
    pConfirmationInterceptor->clearConnection();

    // 2nd Connection
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    ConnectionInternal* pConnectionInternalPtr2 = pConnection2.unsafeCast<ConnectionInternal>();
    pConnection2 = NULL;
    pConfirmationInterceptor->clearConnection();

    // 3rd Connection
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient->newCall(pRequest3);
    Response::Ptr pResponse3 = pCall3->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    Connection::Ptr pConnection3 = pConfirmationInterceptor->getConnection();
    ConnectionInternal* pConnectionInternalPtr3 = pConnection3.unsafeCast<ConnectionInternal>();
    pConnection3 = NULL;
    pConfirmationInterceptor->clearConnection();

    // 4th Connection
    Request::Builder requestBuilder4;
    Request::Ptr pRequest4 = requestBuilder4.setUrl(url).build();
    Call::Ptr pCall4 = pHttpClient->newCall(pRequest4);
    Response::Ptr pResponse4 = pCall4->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse4->getCode());
    Connection::Ptr pConnection4 = pConfirmationInterceptor->getConnection();
    ConnectionInternal* pConnectionInternalPtr4 = pConnection4.unsafeCast<ConnectionInternal>();
    pConnection4 = NULL;
    pConfirmationInterceptor->clearConnection();

    // read and close 1st Connection
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream1 = pResponseBody1->getByteStream();
    Poco::Buffer<char> responseBodyBuffer1(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream1, responseBodyBuffer1);
    pResponseBodyStream1->close();

    // read and close 2nd Connection
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    pResponseBodyStream2->close();

    // read and close 3rd Connection
    ResponseBody::Ptr pResponseBody3 = pResponse3->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream3 = pResponseBody3->getByteStream();
    Poco::Buffer<char> responseBodyBuffer3(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream3, responseBodyBuffer3);
    pResponseBodyStream3->close();

    ASSERT_EQ(4, pConnectionPool->getTotalConnectionCount());
    ASSERT_EQ(3, pConnectionPool->getKeepAliveIdleConnectionCount());

    // When: read and close 4th Connection response body
    ResponseBody::Ptr pResponseBody4 = pResponse4->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream4 = pResponseBody4->getByteStream();
    Poco::Buffer<char> responseBodyBuffer4(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream4, responseBodyBuffer4);
    pResponseBodyStream4->close();

    // Then: Oldest Connection is removed from ConnectionPool.
    EXPECT_EQ(3, pConnectionPool->getTotalConnectionCount());
    EXPECT_EQ(3, pConnectionPool->getKeepAliveIdleConnectionCount());
    EXPECT_FALSE(static_cast<ConnectionPoolInternal*>(pConnectionPool.get())->
            isConnectionExisting(pConnectionInternalPtr1));
    EXPECT_TRUE(static_cast<ConnectionPoolInternal*>(pConnectionPool.get())->
            isConnectionExisting(pConnectionInternalPtr2));
    EXPECT_TRUE(static_cast<ConnectionPoolInternal*>(pConnectionPool.get())->
            isConnectionExisting(pConnectionInternalPtr3));
    EXPECT_TRUE(static_cast<ConnectionPoolInternal*>(pConnectionPool.get())->
            isConnectionExisting(pConnectionInternalPtr4));
}

// max keep-alive idle count = 0
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest, close_NotAddConnection_WhenMaxKeepAliveIdleCountIsZero)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool with max Keep-Alive idle count is 0.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool(0, 60);
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_EQ(1, pConnectionPool->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPool->getKeepAliveIdleCountMax());

    // Then: read and close
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    // Then: Connection is not added to ConnectionPool
    ASSERT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

// keep-alive timeout = 0 sec
TEST_F(ResponseBodyStreamWithConnectionPoolIntegrationTest, close_NotAddConnection_WhenKeepAliveTimeoutSecIsZero)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool with Keep-Alive timeout sec is 0.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool(3, 0);
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_EQ(1, pConnectionPool->getTotalConnectionCount());

    // Then: read and close
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    // Then: Connection is not added to ConnectionPool
    ASSERT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

} /* namespace test */
} /* namespace easyhttpcpp */
