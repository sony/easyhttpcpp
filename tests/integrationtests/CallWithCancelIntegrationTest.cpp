/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Path.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "EasyHttpCppAssertions.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "TestPreferences.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::HttpTestRequestHandlerFactory;
using easyhttpcpp::testutil::MockInterceptor;
using easyhttpcpp::testutil::TestPreferences;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithCancelIntegrationTest";
static const char* const ContentTypeOctetStream = "application/octet-stream";
static const int TestFailureTimeout = 10 * 1000; // milliseconds
static const size_t ResponseBufferBytes = 8192;
static const char* const CancelResponseBodyUnit = "1234567890";
static const int MultiThreadCount = 10;
static const int CancelResponseBodyUnitCount = 10;
static const int CancelResponseBodyReadUnitSize = 10;
static const size_t LargeBinaryDataBytes = 5000;
static const long ResponseBodyReadInterval = 10;  // 10ms
static const char* const ExternalUrl =
        "http://d3d9bizloqaofq.cloudfront.net/1008_BINARY/bin/data_5000_1441009016.bin";
static const int ThreadPoolMinCapacity = 2;

namespace {

class ForCancelRequestHandler : public Poco::Net::HTTPRequestHandler, public Poco::RefCountedObject {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(CancelResponseBodyUnit) * CancelResponseBodyUnitCount);
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::ostream& ostr = response.send();
        for (int i = 0; i < CancelResponseBodyUnitCount; i++) {
            ostr << CancelResponseBodyUnit;
        }
    }
};

class HttpCancelRunner : public Poco::Runnable, public Poco::RefCountedObject {
public:

    HttpCancelRunner(Call::Ptr pCall, const std::string& runnerId) : m_pCall(pCall), m_result(false),
            m_runnerId(runnerId), m_sendRequestException(false), m_receiveResponseException(false),
            m_pocoException(false), m_executeElapsedTime(0)
    {
    }

    virtual void run()
    {
        m_result = execute();
    }

    // test の verify (EXPECT_XXX) で表示するメッセージを設定します。
    void setErrorMessage(const std::string& message)
    {
        EASYHTTPCPP_LOG_E(Tag, "%s", message.c_str());
        if (m_message.empty()) {
            m_message = message;
        }
    }

    std::string getErrorMessage()
    {
        return m_message;
    }

    bool cancel()
    {
        if (!m_pCall) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] Call is not created.", m_runnerId.c_str());
            return false;
        }
        m_pCall->cancel();
        return true;
    }

    bool getResult()
    {
        return m_result;
    }

    bool isSendRequestException() const
    {
        return m_sendRequestException;
    }

    bool isReceiveResponseException() const
    {
        return m_receiveResponseException;
    }

    bool isPocoException() const
    {
        return m_pocoException;
    }
    Poco::Timestamp::TimeDiff getExecuteElapsedTime()
    {
        return m_executeElapsedTime;
    }

protected:
    virtual bool execute() = 0;

    void checkException(const HttpException& e)
    {
        if (e.getMessage().find("send") != std::string::npos) {
            m_sendRequestException = true;
        }
        if (e.getMessage().find("receive") != std::string::npos) {
            m_receiveResponseException = true;
        }
        if (!e.getCause().isNull()) {
            m_pocoException = true;
        }
    }

protected:
    std::string m_message;
    Call::Ptr m_pCall;
    bool m_result;
    std::string m_runnerId;
    bool m_sendRequestException;
    bool m_receiveResponseException;
    bool m_pocoException;
    Poco::Timestamp m_startTime;
    Poco::Timestamp::TimeDiff m_executeElapsedTime;
};

class HttpCancelRunnerWaitBeforeExecute : public HttpCancelRunner {
public:
    HttpCancelRunnerWaitBeforeExecute(Call::Ptr pCall, const std::string& runnerId) :
            HttpCancelRunner(pCall, runnerId)
    {
    }

    bool waitBeforeExecute()
    {
        return m_beforeExecute.tryWait(TestFailureTimeout);
    }

    void setStartExecute()
    {
        m_startExecute.set();
    }

protected:

    virtual bool execute()
    {
        Response::Ptr pResponse;
        m_beforeExecute.set();
        // wait to execute
        if (!m_startExecute.tryWait(TestFailureTimeout)) {
            setErrorMessage(StringUtil::format("[%s] startExecute is timeout.", m_runnerId.c_str()));
            return false;
        }

        try {
            // execute GET method.
            m_startTime.update();
            pResponse = m_pCall->execute();
        } catch (const HttpExecutionException& e) {
            m_executeElapsedTime = m_startTime.elapsed();
            EASYHTTPCPP_LOG_D(Tag, "[%s] execute: HttpExecutionException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpSslException& e) {
            m_executeElapsedTime = m_startTime.elapsed();
            EASYHTTPCPP_LOG_D(Tag, "[%s] execute: HttpSslException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpException& e) {
            m_executeElapsedTime = m_startTime.elapsed();
            setErrorMessage(StringUtil::format("[%s] execute: exception occurred.code=%u message=%s",
                    m_runnerId.c_str(), e.getCode(), e.getMessage().c_str()));
            checkException(e);
            return false;
        }
        m_executeElapsedTime = m_startTime.elapsed();

        ResponseBody::Ptr pResponseBody = pResponse->getBody();
        ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
        Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
        try {
            while (!pResponseBodyStream->isEof()) {
                pResponseBodyStream->read(responseBodyBuffer.begin(), CancelResponseBodyReadUnitSize);
            }
        } catch (const HttpExecutionException& e) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] read: HttpExecutionException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpIllegalStateException& e) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] read: HttpIllegalException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpSslException& e) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] read: HttpSslException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpException& e) {
            setErrorMessage(StringUtil::format("[%s] read: exception occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str()));
            checkException(e);
            return false;
        }
        return true;
    }

protected:
    Poco::Event m_beforeExecute;
    Poco::Event m_startExecute;
};

class HttpCancelRunnerWaitBeforeReadResponseBody : public HttpCancelRunner {
public:
    HttpCancelRunnerWaitBeforeReadResponseBody(Call::Ptr pCall,
            const std::string& runnerId) : HttpCancelRunner(pCall, runnerId)
    {
    }

    bool waitBeforeReadResponse()
    {
        return m_beforeReadResponse.tryWait(TestFailureTimeout);
    }

    void setStartReadResponse()
    {
        m_startReadResponse.set();
    }

protected:
    virtual bool execute()
    {
        Response::Ptr pResponse;
        // execute
        pResponse = m_pCall->execute();
        EASYHTTPCPP_LOG_D(Tag, "[%s] Response code=%d", m_runnerId.c_str(), pResponse->getCode());

        ResponseBody::Ptr pResponseBody = pResponse->getBody();
        ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();

        m_beforeReadResponse.set();
        // wait to read response body
        if (!m_startReadResponse.tryWait(TestFailureTimeout)) {
            setErrorMessage(StringUtil::format("[%s] startReadResponse is timeout.", m_runnerId.c_str()));
            return false;
        }

        Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
        try {
            while (!pResponseBodyStream->isEof()) {
                Poco::Thread::sleep(ResponseBodyReadInterval);
                pResponseBodyStream->read(responseBodyBuffer.begin(), CancelResponseBodyReadUnitSize);
            }
        } catch (const HttpExecutionException& e) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] read: HttpExecutionException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpIllegalStateException& e) {
            EASYHTTPCPP_LOG_D(Tag, "[%s] read: HttpIllegalException occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str());
            checkException(e);
            return true;
        } catch (const HttpException& e) {
            setErrorMessage(StringUtil::format("[%s] read: exception occurred.code=%u message=%s", m_runnerId.c_str(),
                    e.getCode(), e.getMessage().c_str()));
            checkException(e);
            return false;
        }
        return true;
    }

private:
    Poco::Event m_beforeReadResponse;
    Poco::Event m_startReadResponse;
};

} /* namespace */

class CallWithCancelIntegrationTest : public HttpIntegrationTestCase {
protected:

    static void SetUpTestCase()
    {
        // initialize test preferences with QA profile
        TestPreferences::getInstance().initialize(TestPreferences::ProfileQA);
    }

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
    Poco::AutoPtr<ForCancelRequestHandler> m_pHandlers[MultiThreadCount];
    Poco::AutoPtr<HttpCancelRunner> m_pExecutes[MultiThreadCount];
    Poco::SharedPtr<ByteArrayBuffer> m_pRequestBuffer[MultiThreadCount];

};

// execute前のcancel (network access)
// exception が throw される。
TEST_F(CallWithCancelIntegrationTest, execute_ThrowsHttpExecutionException_WhenCallCancelBeforeExecute)
{
    // Given: cancel before Call::execute
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // cancel
    pCall->cancel();

    // When: call execute
    // Then: throw execption
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpExecutionException, 100702);
}

// execute前のcancel (cache 使用)
// exception が throw される。
TEST_F(CallWithCancelIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenCallCancelBeforeExecuteWithCachedResponse)
{
    // Given: exist in cache and cancel before Call::execute and use cached response.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // call cancel
    pCall2->cancel();

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pCall2->execute(), HttpExecutionException, 100702);
}

// cancel 後の response body 受信
// exception が throw される。
TEST_F(CallWithCancelIntegrationTest, read_ThrowsHttpIllegalStateException_WhenCallCancelBeforeReadResponseBody)
{
    // Given: cancel before read response body.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Response::Ptr pResponse = pCall->execute();
    EXPECT_TRUE(pResponse->isSuccessful());

    // call cancel
    pCall->cancel();

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: read response body
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// cancel 後の cache の response body 受信で
// exception が throw される。
TEST_F(CallWithCancelIntegrationTest, read_ThrowsHttpIllegalStateException_WhenCallCancelBeforeReadCachedResponseBody)
{
    // Given: exist in cache and cancel before read response from cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // call cancel
    pCall2->cancel();

    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // When: read response body.
    // Then: throw exception.
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream2->read(responseBodyBuffer2.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// cancel を２回呼び出す。
// アクセス違反等はっせいしない。
TEST_F(CallWithCancelIntegrationTest, cancel_ReturnsTrue_WhenCallCancelOfSecondTimeAfterExecute)
{
    // Given: cancel before read response body.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Response::Ptr pResponse = pCall->execute();
    EXPECT_TRUE(pResponse->isSuccessful());

    // When cancel 2 times
    // Then: 2nd cancel returns true because Connection was removed.
    EXPECT_TRUE(pCall->cancel());
    EXPECT_TRUE(pCall->cancel());

    // read response body throw exception
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// sendRequest (execute) 中の cancel 呼び出しタイミングテスト。(Network access)
// Cancel を受け付けると、HttpExecutionException がthrow される。
TEST_F(CallWithCancelIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenAcceptedCancelDuringNetworkAccess)
{
    // Given: start execute on multi thread and wait before execute
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    Poco::ThreadPool threadPool(ThreadPoolMinCapacity, MultiThreadCount);
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new ForCancelRequestHandler;
        std::string handlerId = Poco::NumberFormatter::format(i);
        std::string path = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, i);
        testServer.getTestRequestHandlerFactory().addHandler(path, m_pHandlers[i], handlerId);
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, path);

        Request::Builder requestBuilder;
        requestBuilder.setUrl(url).setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handlerId);

        m_pRequestBuffer[i] = new ByteArrayBuffer(LargeBinaryDataBytes);
        MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
        m_pRequestBuffer[i]->setWrittenDataSize(LargeBinaryDataBytes);
        Byte* pBuffer = m_pRequestBuffer[i]->getBuffer();
        for (size_t j = 0; j < LargeBinaryDataBytes; j++) {
            pBuffer[j] = (j & 0xff);
        }

        RequestBody::Ptr pRequestBody;
        pRequestBody = RequestBody::create(pMediaType, *(m_pRequestBuffer[i]));
        requestBuilder.httpPost(pRequestBody);

        Request::Ptr pRequest = requestBuilder.build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        m_pExecutes[i] = new HttpCancelRunnerWaitBeforeExecute(pCall, handlerId);
        threadPool.start(*m_pExecutes[i]);
    }

    // sync execute
    // wait before execute
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(static_cast<HttpCancelRunnerWaitBeforeExecute*>(m_pExecutes[i].get())->waitBeforeExecute());
    }

    // start execute
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpCancelRunnerWaitBeforeExecute*>(m_pExecutes[i].get())->setStartExecute();
    }

    // When: cancel
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(m_pExecutes[i]->cancel());
    }

    threadPool.stopAll();

    // Then: throw exception when accepted cancel.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->getResult()) << m_pExecutes[i]->getErrorMessage();
    }
}

// sendRequest (execute) 中の cancel 呼び出しタイミングテスト。(External server)
// Cancel を受け付けると、HttpExecutionException がthrow される。
TEST_F(CallWithCancelIntegrationTest,
        execute_ThrowsHttpExecutionException_WhenAcceptedCancelWithExternalServer)
{
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // Given: set proxy
    EasyHttp::Builder httpClientBuilder;
    Proxy::Ptr pProxy(TestPreferences::getInstance().optGetProxy(NULL));
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).setCache(pCache).build();
    unsigned int abortTimeoutSec = pHttpClient->getTimeoutSec() - 5; // connect hang チェックは、timeout - 5sec とする

    int totalSendRequestExceptionCount = 0;
    int totalReceiveResponseExceptionCount = 0;
    int totalSendRequestExceptionFromPocoCount = 0;
    int totalReceiveResponseExceptionFromPocoCount = 0;

    // sendRequest と cancel をほぼ同時に実行させるのが難しい。
    // setStartExecute() の後、すぐcancelすると、HTTPCLientSession 生成前にcancelが呼び出されていた。
    // cancel の呼び出しタイミングを 50ms ずつずらしてテストをじっこうしたところ
    // 0～500ms の間に、1,2回 sendRequest 中の cancel が確認できた。
    // 全然はっせいしない時もあります。
    // PC 等の環境が違うと、このタイミングは、見直す必要がでてくると思います。
    for (long sleepMilliSec = 0; sleepMilliSec < 500; sleepMilliSec += 50) {
        int sendRequestExceptionCount = 0;
        int receiveResponseExceptionCount = 0;
        int sendRequestExceptionFromPocoCount = 0;
        int receiveResponseExceptionFromPocoCount = 0;

        Poco::ThreadPool threadPool(ThreadPoolMinCapacity, MultiThreadCount);
        for (int i = 0; i < MultiThreadCount; i++) {
            std::string runnerId = Poco::NumberFormatter::format(i);

            Request::Builder requestBuilder;
            requestBuilder.setUrl(ExternalUrl);

            Request::Ptr pRequest = requestBuilder.build();
            Call::Ptr pCall = pHttpClient->newCall(pRequest);

            m_pExecutes[i] = new HttpCancelRunnerWaitBeforeExecute(pCall, runnerId);
            threadPool.start(*m_pExecutes[i]);
        }

        // sync execute
        // wait before execute
        for (int i = 0; i < MultiThreadCount; i++) {
            ASSERT_TRUE(static_cast<HttpCancelRunnerWaitBeforeExecute*>(m_pExecutes[i].get())->waitBeforeExecute());
        }

        // start execute
        for (int i = 0; i < MultiThreadCount; i++) {
            static_cast<HttpCancelRunnerWaitBeforeExecute*>(m_pExecutes[i].get())->setStartExecute();
        }

        // When: cancel
        for (int i = 0; i < MultiThreadCount; i++) {
            Poco::Thread::sleep(sleepMilliSec);
            ASSERT_TRUE(m_pExecutes[i]->cancel());
        }

        threadPool.stopAll();

        // Then: throw exception when accepted cancel.
        for (int i = 0; i < MultiThreadCount; i++) {
            ASSERT_TRUE(m_pExecutes[i]->getResult()) << m_pExecutes[i]->getErrorMessage();
            if (m_pExecutes[i]->isSendRequestException()) {
                sendRequestExceptionCount++;
                if (m_pExecutes[i]->isPocoException()) {
                    sendRequestExceptionFromPocoCount++;
                }
            }
            if (m_pExecutes[i]->isReceiveResponseException()) {
                receiveResponseExceptionCount++;
                if (m_pExecutes[i]->isPocoException()) {
                    receiveResponseExceptionFromPocoCount++;
                }
            }
            // in the case of more than abortTimeoutSec, we thought that was hung.
            EXPECT_GT(abortTimeoutSec, m_pExecutes[i]->getExecuteElapsedTime() / 1000000)  << "may be connection hang.";
        }

        EASYHTTPCPP_LOG_D(Tag, "Exception from sendRequest = %d (fromPoco = %d) sleep=%ld", sendRequestExceptionCount,
                sendRequestExceptionFromPocoCount, sleepMilliSec);
        EASYHTTPCPP_LOG_D(Tag, "Exception from receiveResponse = %d (fromPoco = %d)", receiveResponseExceptionCount,
                receiveResponseExceptionFromPocoCount);

        totalSendRequestExceptionCount += sendRequestExceptionCount;
        totalReceiveResponseExceptionCount += receiveResponseExceptionCount;
        totalSendRequestExceptionFromPocoCount += sendRequestExceptionFromPocoCount;
        totalReceiveResponseExceptionFromPocoCount += receiveResponseExceptionFromPocoCount;
    }

    EASYHTTPCPP_LOG_D(Tag, "total Exception from sendRequest = %d (fromPoco = %d)", totalSendRequestExceptionCount,
            totalSendRequestExceptionFromPocoCount);
    EASYHTTPCPP_LOG_D(Tag, "total Exception from receiveResponse = %d (fromPoco = %d)", totalReceiveResponseExceptionCount,
            totalReceiveResponseExceptionFromPocoCount);
}

// ResponseBody のよみこみ中の、cancel 呼び出しタイミングテスト。(Network access)
// Cancel を受け付けると、HttpIllegalStateException がthrow される
TEST_F(CallWithCancelIntegrationTest,
        read_ThrowsHttpIllegalStateException_WhenAcceptedCancelDuringReadFromNetwork)
{
    // Given: start execute on multi thread and wait before read response
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    Poco::ThreadPool threadPool(ThreadPoolMinCapacity, MultiThreadCount);
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new ForCancelRequestHandler;
        std::string handlerId = Poco::NumberFormatter::format(i);
        std::string path = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, i);
        testServer.getTestRequestHandlerFactory().addHandler(path, m_pHandlers[i], handlerId);
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, path);

        Request::Builder requestBuilder;
        requestBuilder.setUrl(url).setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handlerId);

        Request::Ptr pRequest = requestBuilder.build();

        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        m_pExecutes[i] = new HttpCancelRunnerWaitBeforeReadResponseBody(pCall, handlerId);
        threadPool.start(*m_pExecutes[i]);
    }

    // sync read response
    // wait before read response
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(static_cast<HttpCancelRunnerWaitBeforeReadResponseBody*>
                (m_pExecutes[i].get())->waitBeforeReadResponse());
    }

    // start read response
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpCancelRunnerWaitBeforeReadResponseBody*>(m_pExecutes[i].get())->setStartReadResponse();
    }

    // When: cancel
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(m_pExecutes[i]->cancel());
        EASYHTTPCPP_LOG_D(Tag, "cancel no=%d", i);
    }

    threadPool.stopAll();

    // Then: throw exception when accepted cancel.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->getResult()) << m_pExecutes[i]->getErrorMessage();
    }
}

// ResponseBody のよみこみ中に、cancel 呼び出しタイミングテスト。(Cache access)
// Cancel を受け付けると、HttpIllegalStateException がthrow される
TEST_F(CallWithCancelIntegrationTest,
        read_ThrowsHttpIllegalStateException_WhenAcceptedCancelDuringReadFromCache)
{
    // Given: start execute on multi thread and wait before read response
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    Poco::ThreadPool threadPool(ThreadPoolMinCapacity, MultiThreadCount);
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new ForCancelRequestHandler;
        std::string handlerId = Poco::NumberFormatter::format(i);
        std::string path = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, i);
        testServer.getTestRequestHandlerFactory().addHandler(path, m_pHandlers[i], handlerId);

        // create cache
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, path, StringUtil::format("id=%s", handlerId.c_str()));

        Request::Builder requestBuilder1;
        Request::Ptr pRequest1 = requestBuilder1.setUrl(url).
                setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handlerId).
                build();
        Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);
        Response::Ptr pResponse = pCall1->execute();
        ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
        // read response body and close
        pResponse->getBody()->toString();


        Request::Builder requestBuilder2;
        requestBuilder2.setUrl(url).setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handlerId);

        Request::Ptr pRequest2 = requestBuilder2.build();

        Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

        // create test thread
        m_pExecutes[i] = new HttpCancelRunnerWaitBeforeReadResponseBody(pCall2, handlerId);
        threadPool.start(*m_pExecutes[i]);
    }

    // sync read response
    // wait before read response
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(static_cast<HttpCancelRunnerWaitBeforeReadResponseBody*>
                (m_pExecutes[i].get())->waitBeforeReadResponse());
    }

    // start read response
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpCancelRunnerWaitBeforeReadResponseBody*>(m_pExecutes[i].get())->setStartReadResponse();
    }

    // When: cancel
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(m_pExecutes[i]->cancel());
    }

    threadPool.stopAll();

    // Then: throw exception when accepted cancel.
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(m_pExecutes[i]->getResult()) << m_pExecutes[i]->getErrorMessage();
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
