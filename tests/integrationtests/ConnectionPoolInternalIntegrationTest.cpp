/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Timestamp.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Interceptor.h"
#include "HeaderContainMatcher.h"
#include "HttpTestServer.h"

#include "CallInternal.h"
#include "ConnectionConfirmationInterceptor.h"
#include "ConnectionInternal.h"
#include "EasyHttpInternal.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const char* const Tag = "ConnectionPoolInternalIntegrationTest";
static const char* const HeaderConnection = "Connection";
static const char* const HeaderValueKeepAlive = "Keep-Alive";

class ConnectionPoolInternalIntegrationTest : public HttpIntegrationTestCase {
};

// temporary の ConnectionPool に、Keep-Alive timeout 待ちがあっても、ConnectionPool のdestructor はすぐに終了する。
TEST_F(ConnectionPoolInternalIntegrationTest,
        destructor_ReturnsWithoutWaitUntilKeepAliveTimeout_WhenExistKeepAliveTimeoutTaskInTemporaryConnectionPool)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    // Given: create EasyHttp without ConnectionPool.
    //        after communication, add KeepAliveTimeoutTask to temporary ConnectionPool.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // response header contains Connection::Keep-Alive.
    ASSERT_THAT(pResponse->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    // read and close response body.
    // connection in temporary connection pool become idle.
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    pResponseBody->toString();

    // get temporary ConnectionPool
    HttpEngine::Ptr pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            pHttpEngine->getConnectionPool().unsafeCast<ConnectionPoolInternal>();

    ConnectionInternal::Ptr pConnectionInternal =
            pConfirmationInterceptor->getConnection().unsafeCast<ConnectionInternal>();
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    ASSERT_TRUE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal));

    // decrement reference count.
    pConfirmationInterceptor->clearConnection();
    pResponseBody = NULL;
    pResponse = NULL;
    pHttpClient = NULL;

    // reference count before release Call instance.
    ASSERT_EQ(2, pConnectionInternal->referenceCount());        // from this function and ConnectionPool
    ASSERT_EQ(2, pConnectionPoolInternal->referenceCount());    // from this function and HttpEngine
    ASSERT_EQ(2, pHttpEngine->referenceCount());                // from this function and Call
    ASSERT_EQ(1, pCall->referenceCount());
    pConnectionInternal = NULL;
    pConnectionPoolInternal = NULL;
    pHttpEngine = NULL;

    // When: execute destructor of ConnectionPool by release Call.
    Poco::Timestamp start;
    pCall = NULL;
    
    // Then: release immediately. (think margin is 500 ms)
    unsigned long long releaseMicroSec = static_cast<unsigned long long>(start.elapsed());
    EASYHTTPCPP_LOG_D(Tag, "ConnectoinPool release time = %zu us", releaseMicroSec);
    EXPECT_GT(static_cast<unsigned long long>(500*1000), releaseMicroSec);
}

// ConnectionPool に Keep-Alive timeout 待ちがあっても、ConnectionPool の destructor はすぐに終了する。
TEST_F(ConnectionPoolInternalIntegrationTest,
        destructor_ReturnsWithoutWaitUntilKeepAliveTimeout_WhenExistKeepAliveTimeoutTaskInConnectionPool)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: create EasyHttp with ConnectionPool
    //        after communication, add KeepAliveTimeoutTask to ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool().unsafeCast<ConnectionPoolInternal>();

    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    EasyHttp::Ptr pHttpClient;
    {
        // place EasyHttp::Builder in the scope because EasyHttp::Builder refer to ConnectionPool.
        EasyHttp::Builder httpClientBuilder;
        pHttpClient = httpClientBuilder.setConnectionPool(pConnectionPoolInternal)
                .addNetworkInterceptor(pNetworkInterceptor).build();
    }
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // response header contains Connection::Keep-Alive.
    ASSERT_THAT(pResponse->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    // read and close response body.
    // connection in connection pool become idle.
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    pResponseBody->toString();

    ConnectionInternal::Ptr pConnectionInternal =
            pConfirmationInterceptor->getConnection().unsafeCast<ConnectionInternal>();
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    ASSERT_TRUE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal));

    // decrement reference count.
    pConfirmationInterceptor->clearConnection();
    EasyHttpContext::Ptr pContext = static_cast<EasyHttpInternal*>(pHttpClient.get())->getHttpContenxt();
    pResponseBody = NULL;
    pResponse = NULL;
    pHttpClient = NULL;
    pCall = NULL;

    // reference count before release ConnectionPool.
    ASSERT_EQ(2, pConnectionInternal->referenceCount());        // from this function and ConnectionPool
    ASSERT_EQ(2, pConnectionPoolInternal->referenceCount());    // from this function and EasyHttpContext
    ASSERT_EQ(1, pContext->referenceCount());
    pContext = NULL;
    pConnectionInternal = NULL;

    // When: execute destructor of ConnectionPool by release ConnectionPool.
    Poco::Timestamp start;
    pConnectionPoolInternal = NULL;
    
    // Then: release immediately. (think margin is 500 ms)
    unsigned long long releaseMicroSec = static_cast<unsigned long long>(start.elapsed());
    EASYHTTPCPP_LOG_D(Tag, "ConnectoinPool release time = %zu us", releaseMicroSec);
    EXPECT_GT(static_cast<unsigned long long>(500*1000), releaseMicroSec);
}

} /* namespace test */
} /* namespace easyhttpcpp */
