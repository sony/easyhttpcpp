/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>

#include "gtest/gtest.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "HttpsTestServer.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

#include "ConnectionConfirmationInterceptor.h"
#include "ConnectionInternal.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "CallInternal.h"

using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::HttpsTestServer;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const int Timeout1Sec = 1;
static const int Timeout3Sec = 3;

namespace {

class CancelRequestHandler : public Poco::Net::HTTPRequestHandler  {
public:
    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        // cancel
        m_pCall->cancel();

        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
    void setCall(Call::Ptr pCall)
    {
        m_pCall = pCall;
    }

private:
    Call::Ptr m_pCall;
};

} /* namespace */

class CallWithRetryByConnectionPoolIntegrationTest : public testing::Test {
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

// 新規 Connection 通信 timeout
// 1. ConnectionPool に再利用可能な Connection なし。
// 2. 通信を timeout にする。
// 確認項目
// retry なしで、HttpTimeoutException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpTimeoutExceptionWithoutRetryConnection_WhenNoReusedConnectionAndReceiveResponseHasTimedOut)
{
    // Given: 通信 Timeout を 1秒にし、response を返さない handler を使用する。
    // ConnectionPool に再利用可能な Connection なし
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp with timeout.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .setTimeoutSec(Timeout1Sec).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: call Call::execute and generate timeout.
    // Then: throw exception and do not retry connection.
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpTimeoutException, 100703);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine->isConnectionRetried());
    handler.set();
}

// 新規 Connection SSL Exception
// 1. ConnectionPool に再利用可能な Connection なし。
// 2. SSL Exception をはっせいさせる。
// 確認項目
// retry なしで、HttpSslException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsSslExceptionWithoutRetryConnection_WhenNoReusedConnectionAndNotSpecifiedRootCa)
{
    // load cert data
    HttpTestUtil::loadDefaultCertData();

    HttpsTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // set cert and start server
    HttpTestUtil::setupHttpsServerDefaultSetting(testServer);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // Given: rootCa の指定なしで、EasyHttp を生成。
    // ConnectionPool に再利用可能な Connection なし
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultHttpsTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: call Call::execute and throw SslException.
    // Then: throw exception and do not retry connection.
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpSslException, 100704);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine->isConnectionRetried());
}

// 新規 Connection cancel
// 1. ConnectionPool に再利用可能な Connection なし。
// 2. 通信中に Call::cancel 呼び出し。
// 確認項目
// retry なしで、HttpExecutionException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpExecutionExceptionWithoutRetryConnection_WhenNoReusedConnectionAndCancelWhileReceiveResponse)
{
    // Given: 通信中に cancel する handler を使用する。
    // ConnectionPool に再利用可能な Connection なし
    HttpTestServer testServer;
    CancelRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    handler.setCall(pCall);

    // When: call Call::execute and cancel で、HttpExecutionException をはっせいさせる。
    // Then: throw exception and do not retry connection
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine->isConnectionRetried());
}

// 新規 Connection IllegalArgument Exception
// 1. ConnectionPool に再利用可能な Connection なし。
// 2. IllegalArgumentException をはっせいさせる (サポートしていない scheme)
// 確認項目
// retry なしで、HttpIllegalArgumentException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpIllegalArgumentExceptionWithoutRetryConnection_WhenNoReusedConnectionAndUnsupportedScheme)
{
    // Given: ConnectionPool に再利用可能な Connection なし
    // サポートしていない scheme
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    Request::Builder requestBuilder;

    std::string url = HttpTestUtil::makeUrl("ftp", HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, HttpTestConstants::DefaultQuery);
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: call Call::execute and throw IllegalArgumentException.
    // Then: throw exception and do not retry connection.
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpIllegalArgumentException, 100700);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine->isConnectionRetried());
}

// 新規 Connection で、Poco の Exception がはっせい → retry なし。
// 1. TestServer を起動しない。
// 2. ConnectionPool に再利用可能な Connection なし。
// 3. http 通信で PocoのExceptionをはっせいさせる。
// 確認項目
// retry なしで、HttpExecutionException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpExecutionExceptionWithoutRetryConnection_WhenNoReusedConnectionAndPocoExceptionOccurredInSendRequest)
{
    // Given: TestServer を起動しない。
    // ConnectionPool に再利用可能な Connection なし
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: call Call::execute and sendRequest で、Poco::Exception がはっせい。
    // Then: throw exception and do not retry connection.
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine->isConnectionRetried());
}

// Connection 再利用時の通信 で timeout
// 1. 一回通信して、Connection を ConnectionPool に追加
// 2. 同じ url で通信。
// 3. handler で timeout にする。
// 4. Connection 再利用の通信でtimeout
// 確認項目
// HttpTimeoutException が throw される。
// retry していない。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpTimeoutException_WhenReusedConnectionIsTimeout)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    testServer.start(HttpTestConstants::DefaultPort);

    // Given: 1 回通信をして、Connection をとうろくする。
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .setTimeoutSec(Timeout1Sec).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();
    HttpEngine* pHttpEngine1 = static_cast<CallInternal*>(pCall1.get())->getHttpEngine();
    Connection::Ptr pConnection1 = pHttpEngine1->getConnection();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // timeout となる handler を使用。、
    HttpTestCommonRequestHandler::WaitRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: call execute
    // Then: throw HttpTimeoutException
    // 再利用された Connection で、retry は行われない。
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall2->execute(), HttpTimeoutException, 100703);
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    Connection::Ptr pConnection2 = pHttpEngine2->getConnection();
    EXPECT_EQ(pConnection1.get(), pConnection2.get());
    EXPECT_FALSE(pHttpEngine2->isConnectionRetried());
    handler2.set();
}

// Connection 再利用でエラー発生 → retry (RequestBody なし)
// 1. server の keep-alive timeout を短くする(1秒)。
// 2. 一回通信して、Connection を ConnectionPool に追加。
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. RequestBody なしで、retry する。
// 確認項目
// 新しい Connection で retry される。
// retry の通信は成功。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ReturnsResponse_WhenWithoutRequestBodyAndReusedConnectionDisconnectsFromServerAndRetryConnectionSucceeds)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // Given: サーバーの keep=Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();
    HttpEngine* pHttpEngine1 = static_cast<CallInternal*>(pCall1.get())->getHttpEngine();
    Connection::Ptr pConnection1 = pHttpEngine1->getConnection();

    handler.clearParameter();

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // request body なし。
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → retry する。
    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // 新しい connection で retry したことを確認。
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    Connection::Ptr pConnection2 = pHttpEngine2->getConnection();
    EXPECT_NE(pConnection1.get(), pConnection2.get());
    EXPECT_TRUE(pHttpEngine2->isConnectionRetried());
}

// Connection 再利用でエラーはっせい → retry (RequestBody あり)
// 1. server の keep-alive timeout を短くする(1秒)。
// 2. 一回通信して、Connection を ConnectionPool に追加。
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. RequestBody は、string のため、retry する。
// 確認項目
// 新しい Connection で retry される。
// retry の通信は成功。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ReturnsResponse_WhenWithRequestBodyAndReusedConnectionDisconnectsFromServerAndRetryConnectionSucceeds)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // Given: サーバーの keep=Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();
    HttpEngine* pHttpEngine1 = static_cast<CallInternal*>(pCall1.get())->getHttpEngine();
    Connection::Ptr pConnection1 = pHttpEngine1->getConnection();

    handler.clearParameter();

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // request body は、string。
    RequestBody::Ptr pRequestBody2;
    MediaType::Ptr pMediaType2(new MediaType(HttpTestConstants::DefaultRequestContentType));
    Poco::SharedPtr<std::string> pContent = new std::string(HttpTestConstants::DefaultRequestBody);
    pRequestBody2 = RequestBody::create(pMediaType2, pContent);
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).httpPut(pRequestBody2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → request body は string なので、retry する。
    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // 新しい connection で retry したことを確認。
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    Connection::Ptr pConnection2 = pHttpEngine2->getConnection();
    EXPECT_NE(pConnection1.get(), pConnection2.get());
    EXPECT_TRUE(pHttpEngine2->isConnectionRetried());
}

// Connection 再利用でエラー → retry で通信 timeout
// 1. server の keep-alive timeout を短くする(1秒)。
// 2. 一回通信して、Connection を ConnectionPool に追加
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. retry の handler で timeout にする。
// 確認項目
// HttpTimeoutException が throw される。
// retry 通信している。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpTimeoutException_WhenReusedConnectionDisconnectsFromServerAndRetryConnectionIsTimeout)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    // Given: サーバーの keep=Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
            .setTimeoutSec(Timeout3Sec).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // 通信 timeout する handler を使用する。
    // 1回目の通信は、サーバー Keep-Alive timeout のため、handler は呼び出されない。
    // retry の通信で、handler が呼び出され、timeout となる。
    HttpTestCommonRequestHandler::WaitRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → retry は handler で timout
    // Then: throw exception and connection retry あり。
    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall2->execute(), HttpTimeoutException, 100703);
    HttpEngine* pHttpEngine = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    EXPECT_TRUE(pHttpEngine->isConnectionRetried());
    handler2.set();
}

// Connection 再利用でエラー → retry 中にcancel
// 1. server の keep-alive timeout を短くする(1秒)
// 2. 一回通信して、Connection を ConnectionPool に追加
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. retry の通信での NetworkInterceptor で Call::cancel 呼び出し。
// 確認項目
// HttpExecutionException が throw される。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenReusedConnectionDisconnectsFromServerAndRetyConnectionIsCancelled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);

    // Given: サーバーの keep=Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    testServer.getTestRequestHandlerFactory().removeHandler(HttpTestConstants::DefaultPath);

    // 通信中に cancel する handler を登録。
    CancelRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2);

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    handler2.setCall(pCall2);

    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → retry の通信中に cancel
    // Then: retry の通信で、HttpExecutionException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall2->execute(), HttpExecutionException, 100702);

    // retry したことを確認。
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    EXPECT_TRUE(pHttpEngine2->isConnectionRetried());
}

// Connection 再利用でエラーはっせい→ retry (RequestBody は ByteBufferArray)
// 1. server の keep-alive timeout を短くする(1秒)
// 2. 一回通信して、Connection を ConnectionPool に追加
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. RequestBody は、ByteArrayBuffer なので、retry 可能。
// 確認項目
// 新しい Connection で retry される。
// retry の通信は成功。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ReturnsResponse_WhenWithRequestBodyByByteArrayBufferAndReusedConnectionDisconnectsFromServerAndRetryConnectionSucceeds)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // Given: サーバーの keep-Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();

    // １回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    handler.clearParameter();

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // request body は、ByteArrayBuffer。
    RequestBody::Ptr pRequestBody2;
    MediaType::Ptr pMediaType2(new MediaType(HttpTestConstants::DefaultRequestContentType));
    Poco::SharedPtr<ByteArrayBuffer> pContent = new ByteArrayBuffer(HttpTestConstants::DefaultRequestBody);
    pRequestBody2 = RequestBody::create(pMediaType2, pContent);
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).httpPut(pRequestBody2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → ByteArrayBuffer のため retry。
    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms
    Response::Ptr pResponse2 = pCall2->execute();

    // Then: retry して通信成功。
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // retry したことを確認。
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    EXPECT_TRUE(pHttpEngine2->isConnectionRetried());
}

// Connection 再利用→エラーはっせい→ retry (RequestBody は std::istream なので retry 不可能)
// 1. server の keep-alive timeout を短くする(1秒)
// 2. 一回通信して、Connection を ConnectionPool に追加
// 3. server の keep-alive timeout 時間が過ぎた後に、同じ url で通信し HttpExecutionException
// 4. RequestBody は、std::istream なので、reset できない。
// 確認項目
// HttpConnectionRetryException が throw される。
// retry しない。
TEST_F(CallWithRetryByConnectionPoolIntegrationTest,
        execute_ThrowsHttpConnectionRetryException_WhenWithRequestBodyByIStreamAndReusedConnectionDisconnectsFromServerAndRetryConnectionCannotResetRequestBody)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // Given: サーバーの keep=Alive timeout を 1 秒にする。
    // 1 回通信をして、Connection をとうろくする。

    long serverKeepAliveTimeoutSec = 1; // 1 sec.
    testServer.setKeepAliveTimeoutSec(serverKeepAliveTimeoutSec);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool).build();

    // 1 回目の通信。
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();
    pResponseBody1->toString();

    handler.clearParameter();

    // 2 回目の通信。
    // 同じ url で通信し、Connection を再利用。
    // request body は、std::istream。
    RequestBody::Ptr pRequestBody2;
    MediaType::Ptr pMediaType2(new MediaType(HttpTestConstants::DefaultRequestContentType));
    Poco::SharedPtr<std::istream> pContent = new std::stringstream(HttpTestConstants::DefaultRequestBody);
    pRequestBody2 = RequestBody::create(pMediaType2, pContent);
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).httpPut(pRequestBody2).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: サーバーの Keep-Alive timeout 後、Call::execute 呼び出し。
    // 再利用された Connection で通信 → Keep-Alive timeout で通信失敗 → std::istream のため retry なし。
    // Then: throw HttpConnectionRetryException
    Poco::Thread::sleep(serverKeepAliveTimeoutSec * 1000 + 500);    // margin 500ms
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall2->execute(), HttpConnectionRetryException, 100705);

    // retry しなかったことを確認。
    HttpEngine* pHttpEngine2 = static_cast<CallInternal*>(pCall2.get())->getHttpEngine();
    EXPECT_FALSE(pHttpEngine2->isConnectionRetried());
}

} /* namespace test */
} /* namespace easyhttpcpp */
