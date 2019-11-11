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
#include "HttpsTestServer.h"
#include "HttpTestServer.h"
#include "TestLogger.h"
#include "TestPreferences.h"

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
using easyhttpcpp::testutil::HttpsTestServer;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::TestPreferences;

namespace easyhttpcpp {
namespace test {

static const char* const HeaderConnection = "Connection";
static const char* const HeaderValueKeepAlive = "Keep-Alive";
static const char* const HeaderValueClose = "Close";
static const size_t ResponseBufferBytes = 100000;

class ResponseBodyStreamWithReusedConnectionIntegrationTest : public HttpIntegrationTestCase {
public:
    static void SetUpTestCase()
    {
        // initialize test preferences with QA profile
        TestPreferences::getInstance().initialize(TestPreferences::ProfileQA);
    }

   void SetUp()
    {
        Poco::Path cachePath(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(cachePath);

        Poco::Path certRootDir(HttpTestUtil::getDefaultCertRootDir());
        FileUtil::removeDirsIfPresent(certRootDir);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

// local server (http)
// Connection:Close
// Connection はConnectionPool から削除される。
TEST_F(ResponseBodyStreamWithReusedConnectionIntegrationTest,
        close_RemovesConnectionFromConnectionPool_WhenReuseConnectionAndResponseContainsConnectionClose)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool.
    // add Connection to ConnectionPool.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();

    // in 1st request, add Connection to ConnectionPool
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ASSERT_THAT(pResponse1->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    pConfirmationInterceptor->clearConnection();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2nd request by same url.
    // set Connection:Close to response header.
    HttpTestCommonRequestHandler::ConnectionCloseRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_THAT(pResponse2->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueClose));

    // use same Connection
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    ASSERT_EQ(pConnection1.get(), pConnection2.get());

    // When: read and close response body stream
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    pResponseBodyStream2->close();

    // Then: Connection is removed from ConnectionPool.
    EXPECT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

// local server (http)
// response に Connection なし
// Connection はConnectionPool に とうろくされたまま。
TEST_F(ResponseBodyStreamWithReusedConnectionIntegrationTest,
        close_DoesNotRemoveConnectionFromConnectionPool_WhenReuseConnectionAndResponseDoesNotContainConnection)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool.
    // add Connection to ConnectionPool.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).build();

    // in 1st request, add Connection to ConnectionPool
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ASSERT_THAT(pResponse1->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    pConfirmationInterceptor->clearConnection();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2nd request by same url.
    // not set response header to Connection.
    HttpTestCommonRequestHandler::NoConnectionRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getHeaders()->has(HeaderConnection));

    // use same Connection
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    ASSERT_EQ(pConnection1.get(), pConnection2.get());

    // When: read and close response body stream
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    pResponseBodyStream2->close();

    // Then: Connection is not removed from ConnectionPool.
    EXPECT_EQ(1, pConnectionPool->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPool->getKeepAliveIdleConnectionCount());
}

// local server (https)
// Connection:Close
// Connection はConnectionPool から削除される。
// Windows (FLUX) では RootCA ファイル、RootCA ディレクトリの設定をサポートしないためテストをスキップします。
TEST_F(ResponseBodyStreamWithReusedConnectionIntegrationTest,
        close_RemovesConnectionFromConnectionPool_WhenSchemeIsHttpsAndReuseConnectionAndResponseContainsConnectionClose)
{
    // load cert data
    HttpTestUtil::loadDefaultCertData();

    HttpsTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;

    // set cert and start server
    HttpTestUtil::setupHttpsServerDefaultSetting(testServer);

    // Given: create EasyHttp with ConnectionPool.
    // add Connection to ConnectionPool.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).setRootCaDirectory(HttpTestUtil::getDefaultRootCaDirectory())
            .build();

    // in 1st request, add Connection to ConnectionPool
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    pConfirmationInterceptor->clearConnection();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2nd request by same url.
    // set Connection:Close to response header.
    HttpTestCommonRequestHandler::ConnectionCloseRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_THAT(pResponse2->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueClose));

    // use same Connection
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    ASSERT_EQ(pConnection1.get(), pConnection2.get());

    // When: read and close response body stream
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    pResponseBodyStream2->close();

    // Then: Connection is removed from ConnectionPool.
    EXPECT_EQ(0, pConnectionPool->getTotalConnectionCount());
}

// local server (https)
// response に Connection なし
// Connection はConnectionPool に とうろくされたまま。
// Windows (FLUX) では RootCA ファイル、RootCA ディレクトリの設定をサポートしないためテストをスキップします。
TEST_F(ResponseBodyStreamWithReusedConnectionIntegrationTest,
        close_DoesNotRemoveConnectionFromConnectionPool_WhenSchemeIsHttpsAndReuseConnectionAndResponseDoesNotContainConnection)
{
    // load cert data
    HttpTestUtil::loadDefaultCertData();

    HttpsTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;

    // set cert and start server
    HttpTestUtil::setupHttpsServerDefaultSetting(testServer);

    // Given: create EasyHttp with ConnectionPool.
    // add Connection to ConnectionPool.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    std::string certRootDir = HttpTestUtil::getDefaultCertRootDir();

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .addNetworkInterceptor(pNetworkInterceptor).setRootCaDirectory(HttpTestUtil::getDefaultRootCaDirectory())
            .build();

    // in 1st request, add Connection to ConnectionPool
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    pConfirmationInterceptor->clearConnection();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2nd request by same url.
    // does not set Connection to response header.
    HttpTestCommonRequestHandler::NoConnectionRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getHeaders()->has(HeaderConnection));

    // use same Connection
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    ASSERT_EQ(pConnection1.get(), pConnection2.get());

    // When: read and close response body stream
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    pResponseBodyStream2->close();

    // Then: Connection is not removed from ConnectionPool.
    EXPECT_EQ(1, pConnectionPool->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPool->getKeepAliveIdleConnectionCount());
}

} /* namespace test */
} /* namespace easyhttpcpp */
