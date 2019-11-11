/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "HttpTestServer.h"
#include "TestLogger.h"

#include "CallInternal.h"
#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "HttpEngine.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const char* const Tag = "CallWithConnectionPoolAndCancelIntegrationTest";
static const size_t ResponseBufferBytes = 8192;
static const int TestFailureTimeout = 10 * 1000; // milliseconds

class CallWithConnectionPoolAndCancelIntegrationTest : public HttpIntegrationTestCase {
protected:
    void SetUp()
    {
        Poco::Path cachePath(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(cachePath);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

// cancel で Connection 削除
//
// Call::execute の後、ResponseBody の read の前に、Call::cancel する。
// 確認項目
// ConnectoinPool から、Connection が削除される。
TEST_F(CallWithConnectionPoolAndCancelIntegrationTest,
        cancel_RemovesConnectionFromConnectionPool_WhenBeforeReadResponseBody)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool().unsafeCast<ConnectionPoolInternal>();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // Given: Call::execute 後、ResponseBodyStream の close はまだ呼び出さない。
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPoolInternal).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    HttpEngine::Ptr pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    ConnectionInternal::Ptr pConnectionInternal = pHttpEngine->getConnection().unsafeCast<ConnectionInternal>();

    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_TRUE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal));

    // When: Call::cancel
    ASSERT_TRUE(pCall->cancel());

    // Then: Connection が ConnectionPool から削除される。
    ASSERT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// releaseConnection で、HttpEngine から Connection のさんしょうが切れている確認。
// (ResponseBody::close で HttpEngine がConnection のさんしょうを解除するため、
// cancel で ConnectionPool のremoveConnection が呼び出さなれない)
//
// 1. ResponseBody を eof まで read してから close
// 2. Call::cancel をする
// 確認項目
//  ConnectionPool から Connection は削除されない。
TEST_F(CallWithConnectionPoolAndCancelIntegrationTest,
        cancel_DoesNotRemoveConnectionFromConnectionPool_WhenAfterCloseResponseBody)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool().unsafeCast<ConnectionPoolInternal>();

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // Given: Call::execute → ResponseBodyStream::read → close
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPoolInternal).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    HttpEngine::Ptr pHttpEngine = static_cast<CallInternal*>(pCall.get())->getHttpEngine();
    ConnectionInternal::Ptr pConnectionInternal = pHttpEngine->getConnection().unsafeCast<ConnectionInternal>();

    // read and close response body stream
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    pResponseBodyStream->close();

    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_TRUE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal));

    // When: Call::cancel
    ASSERT_TRUE(pCall->cancel());

    // Then: Connection は、ConnectionPool から削除されない。
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_TRUE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal));
}

namespace {

class TestRunner : public Poco::Runnable, public Poco::RefCountedObject {
public:
    TestRunner()
    {
    }

    virtual void run()
    {
        m_startEvent.set();
        m_executeEvent.tryWait(TestFailureTimeout);
        executeProcessing();
    }

    virtual void executeProcessing() = 0;

    bool waitToStart()
    {
        return m_startEvent.tryWait(TestFailureTimeout);
    }

    void execute()
    {
        m_executeEvent.set();
    }

private:
    Poco::Event m_startEvent;
    Poco::Event m_executeEvent;
};

class CancelRunner : public TestRunner {
public:
    typedef Poco::AutoPtr<CancelRunner> Ptr;

    CancelRunner(Call::Ptr pCall) : m_pCall(pCall)
    {
    }

    virtual void executeProcessing()
    {
        EASYHTTPCPP_TESTLOG_D(Tag, "CancelRunner: call cancel()");
        m_pCall->cancel();
        EASYHTTPCPP_TESTLOG_D(Tag, "CancelRunner: cancel() called");
    }

private:
    Call::Ptr m_pCall;
};

class CloseRunner : public TestRunner {
public:
    typedef Poco::AutoPtr<CloseRunner> Ptr;

    CloseRunner(ResponseBodyStream::Ptr pResponseBodyStream) : m_pResponseBodyStream(pResponseBodyStream)
    {
    }

    virtual void executeProcessing()
    {
        EASYHTTPCPP_TESTLOG_D(Tag, "CloseRunner: call close()");
        m_pResponseBodyStream->close();
        EASYHTTPCPP_TESTLOG_D(Tag, "CloseRunner: close() called");
    }

private:
    ResponseBodyStream::Ptr m_pResponseBodyStream;
};

}   // namespace

// Call::cancel() と ResponseBodyStream::close() が別々のスレッドから同時に実行されたときのテスト
// dead lock しないことの確認
// よいタイミングで同時実行するために、10 回テストを繰り返す。
TEST_F(CallWithConnectionPoolAndCancelIntegrationTest, cancel_FinishesNormaly_WhenCancelAndCloseRunInParallel)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Execute 10 times and confirm that it does not dead lock.
    for (int loopCount = 0; loopCount < 10; loopCount++) {
        EASYHTTPCPP_TESTLOG_D(Tag, "start test[%d]", loopCount);

        // Given: create EasyHttp with ConnectionPool.
        ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

        std::string cachePath = HttpTestUtil::getDefaultCachePath();
        HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);
        // clear cache.
        pCache->evictAll();

        // create EasyHttp
        EasyHttp::Builder httpClientBuilder;
        EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).setConnectionPool(pConnectionPool)
                .build();

        // in 1st request, add Connection to ConnectionPool
        std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        Response::Ptr pResponse = pCall->execute();

        ResponseBody::Ptr pResponseBody = pResponse->getBody();
        ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
        Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
        HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);

        Poco::ThreadPool threadPool;
        CancelRunner::Ptr pCancelRunner = new CancelRunner(pCall);
        threadPool.start(*pCancelRunner);
        CloseRunner::Ptr pCloseRunner = new CloseRunner(pResponseBodyStream);
        threadPool.start(*pCloseRunner);

        // When: Parallel execution of Call::cancel and ResponseBodyStream::close.
        ASSERT_TRUE(pCancelRunner->waitToStart());
        ASSERT_TRUE(pCloseRunner->waitToStart());
        pCancelRunner->execute();
        pCloseRunner->execute();

        // Then: success normaly (do not dead lock)
        threadPool.joinAll();
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
