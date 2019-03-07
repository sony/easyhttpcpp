/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "HttpTestServer.h"
#include "EasyHttpCppAssertions.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestResponseCallback.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const int TestFailureTimeout = 10 * 1000; // milliseconds

namespace {

class SynchronousRequest : public Poco::Runnable, public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<SynchronousRequest> Ptr;

    SynchronousRequest(Call::Ptr pCall) : m_pCall(pCall)
    {
    }

    virtual void run()
    {
        m_pResponse = m_pCall->execute();
    }

    Response::Ptr getResponse()
    {
        return m_pResponse;
    }

private:
    Call::Ptr m_pCall;
    Response::Ptr m_pResponse;
};

} /* namespace */

class EasyHttpIntegrationTest : public HttpIntegrationTestCase {
};

TEST_F(EasyHttpIntegrationTest, destructor_CancelsAsynchronousRequest_WhenAsynchronousRequestsIsInProgress)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);
    ASSERT_TRUE(handler.waitForStart(TestFailureTimeout));

    // When: decrement EasyHttp reference counter.
    Poco::Timestamp beforeReleaseTime;
    pHttpClient = NULL;
    Poco::Timestamp afterReleaseTime;

    // Then: destructor 呼び出しの間に、非同期要求が cancel される。
    handler.set();
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_LT(beforeReleaseTime, pCallback->getCompletionTime());
    EXPECT_GT(afterReleaseTime, pCallback->getCompletionTime());
    // onFailure is called and what is HttpExecutionException.
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
}

TEST_F(EasyHttpIntegrationTest, destructor_CancelsAllAsynchronousTask_WhenWaitingAsynchronousRequestIsInQueue)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // Given: 6個の executeAsync 呼び出しを行う。
    // 5個の executeAsync で、thread 数を max にする。
    std::vector<HttpTestCommonRequestHandler::WaitRequestHandler::Ptr> handlers;
    std::vector<Call::Ptr> calls;
    std::vector<HttpTestResponseCallback::Ptr> callbacks;
    int inAdvanceCallCount = 5;
    for (int i = 0; i < inAdvanceCallCount; i++) {
        std::string path = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, i);
        HttpTestCommonRequestHandler::WaitRequestHandler::Ptr pHandler =
                new HttpTestCommonRequestHandler::WaitRequestHandler();
        testServer.getTestRequestHandlerFactory().addHandler(path, pHandler);
        handlers.push_back(pHandler);

        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, path);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);
        calls.push_back(pCall);

        HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
        callbacks.push_back(pCallback);

        pCall->executeAsync(pCallback);
        ASSERT_TRUE(pHandler->waitForStart(TestFailureTimeout));
    }

    // 6個目は、executeAsync は queue に格納される。
    std::string path6 = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, inAdvanceCallCount);
    HttpTestCommonRequestHandler::WaitRequestHandler::Ptr pHandler6 =
            new HttpTestCommonRequestHandler::WaitRequestHandler();
    testServer.getTestRequestHandlerFactory().addHandler(path6, pHandler6);
    handlers.push_back(pHandler6);

    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, path6);
    Request::Builder requestBuilder6;
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).httpGet().build();
    Call::Ptr pCall6 = pHttpClient->newCall(pRequest6);
    calls.push_back(pCall6);

    HttpTestResponseCallback::Ptr pCallback6 = new HttpTestResponseCallback();
    callbacks.push_back(pCallback6);

    pCall6->executeAsync(pCallback6);
    // 実行されない。1秒だけ待ちます。
    ASSERT_FALSE(pHandler6->waitForStart(1000L));

    // When: decrement EasyHttp reference counter.
    Poco::Timestamp beforeReleaseTime;
    pHttpClient = NULL;
    Poco::Timestamp afterReleaseTime;

    // Then: destructor 呼び出しの間に、6個全ての非同期要求が cancel される。
    for (int i = 0; i <= inAdvanceCallCount; i++) {
        // cancel できなかったときのために、handler の wait を解除する。
        handlers[i]->set();
    }
    for (int i = 0; i <= inAdvanceCallCount; i++) {
        EXPECT_TRUE(callbacks[i]->waitCompletion()) << StringUtil::format("id=%u", i);
        EXPECT_LT(beforeReleaseTime, callbacks[i]->getCompletionTime()) << StringUtil::format("id=%u", i);
        EXPECT_GT(afterReleaseTime, callbacks[i]->getCompletionTime()) << StringUtil::format("id=%u", i);
        // onFailure is called and what is HttpExecutionException.
        EXPECT_TRUE(callbacks[i]->getResponse().isNull()) << StringUtil::format("id=%u", i);
        HttpException::Ptr pWhat = callbacks[i]->getWhat();
        ASSERT_FALSE(pWhat.isNull()) << StringUtil::format("id=%u", i);
        EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL) << StringUtil::format("id=%u", i);
    }
}

TEST_F(EasyHttpIntegrationTest, destructor_DoesNotCancelsExecuteMethod_WhenExecuteMethodIsInProgress)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: worker thread で、execute を実行する。.
    Poco::Thread workerThread;
    SynchronousRequest::Ptr pSynchronousRequest = new SynchronousRequest(pCall);
    workerThread.start(*pSynchronousRequest);
    ASSERT_TRUE(handler.waitForStart(TestFailureTimeout));

    // When: decrement EasyHttp reference counter.
    pHttpClient = NULL;

    // Then: execute は、cancel されない。
    handler.set();
    ASSERT_TRUE(workerThread.tryJoin(TestFailureTimeout));
    Response::Ptr pResponse = pSynchronousRequest->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(EasyHttpIntegrationTest, invalidateAndCancel_CancelsAsynchronousRequest_WhenAsynchronousRequestsIsInProgress)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);
    ASSERT_TRUE(handler.waitForStart(TestFailureTimeout));

    // When: call EasyHttp::invalidateAndCancel().
    Poco::Timestamp beforeReleaseTime;
    pHttpClient->invalidateAndCancel();
    Poco::Timestamp afterReleaseTime;

    // Then: invalidateAndCancel 呼び出しの間に、非同期要求が cancel される。
    handler.set();
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_LT(beforeReleaseTime, pCallback->getCompletionTime());
    EXPECT_GT(afterReleaseTime, pCallback->getCompletionTime());
    // onFailure is called and what is HttpExecutionException.
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
}

TEST_F(EasyHttpIntegrationTest, invalidateAndCancel_CancelsAllAsynchronousTask_WhenWaitingAsynchronousRequestIsInQueue)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // Given: 6個の executeAsync 呼び出しを行う。
    // 5個の executeAsync で、thread 数を max にする。
    std::vector<HttpTestCommonRequestHandler::WaitRequestHandler::Ptr> handlers;
    std::vector<Call::Ptr> calls;
    std::vector<HttpTestResponseCallback::Ptr> callbacks;
    int inAdvanceCallCount = 5;
    for (int i = 0; i < inAdvanceCallCount; i++) {
        std::string path = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, i);
        HttpTestCommonRequestHandler::WaitRequestHandler::Ptr pHandler =
                new HttpTestCommonRequestHandler::WaitRequestHandler();
        testServer.getTestRequestHandlerFactory().addHandler(path, pHandler);
        handlers.push_back(pHandler);

        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, path);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);
        calls.push_back(pCall);

        HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
        callbacks.push_back(pCallback);

        pCall->executeAsync(pCallback);
        ASSERT_TRUE(pHandler->waitForStart(TestFailureTimeout));
    }

    // 6個目は、executeAsync は queue に格納される。
    std::string path6 = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, inAdvanceCallCount);
    HttpTestCommonRequestHandler::WaitRequestHandler::Ptr pHandler6 =
            new HttpTestCommonRequestHandler::WaitRequestHandler();
    testServer.getTestRequestHandlerFactory().addHandler(path6, pHandler6);
    handlers.push_back(pHandler6);

    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, path6);
    Request::Builder requestBuilder6;
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).httpGet().build();
    Call::Ptr pCall6 = pHttpClient->newCall(pRequest6);
    calls.push_back(pCall6);

    HttpTestResponseCallback::Ptr pCallback6 = new HttpTestResponseCallback();
    callbacks.push_back(pCallback6);

    pCall6->executeAsync(pCallback6);
    // 実行されない。1秒だけ待ちます。
    ASSERT_FALSE(pHandler6->waitForStart(1000L));

    // When: call EasyHttp::invalidateAndCancel().
    Poco::Timestamp beforeReleaseTime;
    pHttpClient->invalidateAndCancel();
    Poco::Timestamp afterReleaseTime;

    // Then: invalidateAndCancel 呼び出しの間に、6個全ての非同期要求が cancel される。
    for (int i = 0; i <= inAdvanceCallCount; i++) {
        // cancel できなかったときのために、handler の wait を解除する。
        handlers[i]->set();
    }
    for (int i = 0; i <= inAdvanceCallCount; i++) {
        EXPECT_TRUE(callbacks[i]->waitCompletion()) << StringUtil::format("id=%u", i);
        EXPECT_LT(beforeReleaseTime, callbacks[i]->getCompletionTime()) << StringUtil::format("id=%u", i);
        EXPECT_GT(afterReleaseTime, callbacks[i]->getCompletionTime()) << StringUtil::format("id=%u", i);
        // onFailure is called and what is HttpExecutionException.
        EXPECT_TRUE(callbacks[i]->getResponse().isNull()) << StringUtil::format("id=%u", i);
        HttpException::Ptr pWhat = callbacks[i]->getWhat();
        ASSERT_FALSE(pWhat.isNull()) << StringUtil::format("id=%u", i);
        EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL) << StringUtil::format("id=%u", i);
    }
}

TEST_F(EasyHttpIntegrationTest, invalidateAndCancel_DoesNotCancelsExecuteMethod_WhenExecuteMethodIsInProgress)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: worker thread で、execute を実行する。.
    Poco::Thread workerThread;
    SynchronousRequest::Ptr pSynchronousRequest = new SynchronousRequest(pCall);
    workerThread.start(*pSynchronousRequest);
    ASSERT_TRUE(handler.waitForStart(TestFailureTimeout));

    // When: call EasyHttp::invalidateAndCancel().
    pHttpClient->invalidateAndCancel();

    // Then: execute は、cancel されない。
    handler.set();
    ASSERT_TRUE(workerThread.tryJoin(TestFailureTimeout));
    Response::Ptr pResponse = pSynchronousRequest->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(EasyHttpIntegrationTest, newCall_ThrowsHttpIllegalStateException_AfterInvalidateEasyHttp)
{
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();

    // Given: invalidate EasyHttp
    pHttpClient->invalidateAndCancel();

    // When: call newCall()
    // Then: newCall() throws HttpIllegalStateException.
    EASYHTTPCPP_EXPECT_THROW(pHttpClient->newCall(pRequest), HttpIllegalStateException, 100701);
}

TEST_F(EasyHttpIntegrationTest, executeAsync_ThrowsHttpIllegalStateException_AfterInvalidateEasyHttp)
{
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();

    // Given: invalidate EasyHttp
    pHttpClient->invalidateAndCancel();

    // When: call executeAsync()
    // Then: executeAsync() throws HttpIllegalStateException.
    EASYHTTPCPP_EXPECT_THROW(pCall->executeAsync(pCallback), HttpIllegalStateException, 100701);
}

TEST_F(EasyHttpIntegrationTest, execute_Succeeds_AfterInvalidateEasyHttp)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: invalidate EasyHttp
    pHttpClient->invalidateAndCancel();

    // When: call execute()
    Response::Ptr pResponse = pCall->execute();

    // Then: execute() succeeds.
    ASSERT_FALSE(pResponse.isNull());
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(EasyHttpIntegrationTest, executeAsync_Succeeds_WhenReinstantiateEasyHttpAfterInvalidateEasyHttp)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();

    // Given: invalidate EasyHttp
    pHttpClient->invalidateAndCancel();

    // When: instantiate EasyHttp again.
    pHttpClient = httpClientBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Then: executeAsync() succeeds.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(EasyHttpIntegrationTest, invalidateAndCancel_Succeeds_WhenEasyHttpIsAlreadyInvalidated)
{
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();

    // Given: invalidate EasyHttp
    pHttpClient->invalidateAndCancel();

    // When: call invalidateAndCancel() again
    // Then: invalidateAndCancel() Succeeds. do not throw any exception or do not crash
    pHttpClient->invalidateAndCancel();
}

} /* namespace test */
} /* namespace easyhttpcpp */
