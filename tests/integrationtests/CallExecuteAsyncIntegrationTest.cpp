/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Buffer.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestResponseCallback.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallExecuteAsyncIntegrationTest";
static const char* const DefaultRequestContentType = "text/plain";
static const std::string DefaultRequestBody = "request body";
static const int TimeoutSec = 1;
static const int TestFailureTimeout = 10 * 1000; // milliseconds
static const size_t ResponseBufferBytes = 8192;
static const char* const TestPath2 = "/path2";

class CallExecuteAsyncIntegrationTest : public HttpIntegrationTestCase {
};

namespace {

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

class AnyExecutionInResponseCallback : public HttpTestResponseCallback {
public:
    typedef Poco::AutoPtr<AnyExecutionInResponseCallback> Ptr;

    AnyExecutionInResponseCallback()
    {
    }

    AnyExecutionInResponseCallback(Call::Ptr pCall) : m_pCall(pCall)
    {
    }

    std::string& getFirstResponseBody()
    {
        return m_firstResponseBody;
    }

    Response::Ptr getSecondResponse()
    {
        return m_pSecondResponse;
    }

    HttpException::Ptr getSecondWhat()
    {
        return m_pSecondWhat;
    }

protected:
    Call::Ptr m_pCall;
    std::string m_firstResponseBody;
    Response::Ptr m_pSecondResponse;
    HttpException::Ptr m_pSecondWhat;
};

class ReadResponseBodyInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    virtual void onResponse(Response::Ptr pResponse)
    {
        if (pResponse.isNull()) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ReadResponseBodyInOnResponseCallback::onResponse: response is NULL.");
        } else if (pResponse->getBody().isNull()) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ReadResponseBodyInOnResponseCallback::onResponse: response body is NULL.");
        } else {
            m_firstResponseBody = pResponse->getBody()->toString();
        }
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class CancelInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    CancelInOnResponseCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onResponse(Response::Ptr pResponse)
    {
        m_pCall->cancel();
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class ExecuteInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteInOnResponseCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onResponse(Response::Ptr pResponse)
    {
        try {
            m_pCall->execute();
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteInOnResponseCallback::onResponse: execute does not throw exception.");
        } catch (const HttpIllegalStateException& e) {
            m_pSecondWhat = e.clone();
        } catch (...) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteInOnResponseCallback::onResponse: execute throws unexpected exception.");
        }
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class ExecuteAsyncInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteAsyncInOnResponseCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onResponse(Response::Ptr pResponse)
    {
        try {
            HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
            m_pCall->executeAsync(pCallback);
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncInOnResponseCallback::onResponse: execute does not throw exception.");
        } catch (const HttpIllegalStateException& e) {
            m_pSecondWhat = e.clone();
        } catch (...) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncInOnResponseCallback::onResponse: execute throws unexpected exception.");
        }
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class ExecuteByOtherCallInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteByOtherCallInOnResponseCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onResponse(Response::Ptr pResponse)
    {
        m_pSecondResponse = m_pCall->execute();
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class ExecuteAsyncByOtherCallInOnResponseCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteAsyncByOtherCallInOnResponseCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onResponse(Response::Ptr pResponse)
    {
        HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
        m_pCall->executeAsync(pCallback);
        if (!pCallback->waitCompletion()) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncByOtherCallInOnResponseCallback::onResponse: executeAsync timed out.");
        } else {
            m_pSecondResponse = pCallback->getResponse();
        }
        HttpTestResponseCallback::onResponse(pResponse);
    }
};

class ExecuteInOnFailureCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteInOnFailureCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onFailure(HttpException::Ptr pWhat)
    {
        try {
            m_pCall->execute();
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteInOnFailureCallback::onFailure: execute does not throw exception.");
        } catch (const HttpIllegalStateException& e) {
            m_pSecondWhat = e.clone();
        } catch (...) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteInOnFailureCallback::onFailure: execute throws unexpected exception.");
        }
        HttpTestResponseCallback::onFailure(pWhat);
    }
};

class ExecuteAsyncInOnFailureCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteAsyncInOnFailureCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onFailure(HttpException::Ptr pWhat)
    {
        try {
            HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
            m_pCall->executeAsync(pCallback);
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncInOnFailureCallback::onFailure: execute does not throw exception.");
        } catch (const HttpIllegalStateException& e) {
            m_pSecondWhat = e.clone();
        } catch (...) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncInOnFailureCallback::onFailure: execute throws unexpected exception.");
        }
        HttpTestResponseCallback::onFailure(pWhat);
    }
};

class ExecuteByOtherCallInOnFailureCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteByOtherCallInOnFailureCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onFailure(HttpException::Ptr pWhat)
    {
        m_pSecondResponse = m_pCall->execute();
        HttpTestResponseCallback::onFailure(pWhat);
    }
};

class ExecuteAsyncByOtherCallInOnFailureCallback : public AnyExecutionInResponseCallback {
public:
    ExecuteAsyncByOtherCallInOnFailureCallback(Call::Ptr pCall) : AnyExecutionInResponseCallback(pCall)
    {
    }

    virtual void onFailure(HttpException::Ptr pWhat)
    {
        HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
        m_pCall->executeAsync(pCallback);
        if (!pCallback->waitCompletion()) {
            EASYHTTPCPP_TESTLOG_E(Tag, "ExecuteAsyncByOtherCallInOnFailureCallback::onFailure: executeAsync timed out.");
        } else {
            m_pSecondResponse = pCallback->getResponse();
        }
        HttpTestResponseCallback::onFailure(pWhat);
    }
};

} /* namespace */

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnResponse_WhenGetMethodAndHttpStatusIsOk)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    // Given: GET method
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_GET, handler.getMethod());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnResponse_WhenPostMethodAndHttpStatusIsOk)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    RequestBody::Ptr pRequestBody;
    MediaType::Ptr pMediaType(new MediaType(DefaultRequestContentType));
    Poco::SharedPtr<std::string> pContent = new std::string(DefaultRequestBody);
    pRequestBody = RequestBody::create(pMediaType, pContent);

    // Given: POST method
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_POST, handler.getMethod());
    EXPECT_EQ(DefaultRequestBody, handler.getRequestBody());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnResponse_WhenPutMethodAndHttpStatusIsOk)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    RequestBody::Ptr pRequestBody;
    MediaType::Ptr pMediaType(new MediaType(DefaultRequestContentType));
    Poco::SharedPtr<std::string> pContent = new std::string(DefaultRequestBody);
    pRequestBody = RequestBody::create(pMediaType, pContent);

    // Given: PUT method
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpPut(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_PUT, handler.getMethod());
    EXPECT_EQ(DefaultRequestBody, handler.getRequestBody());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnResponse_WhenDeleteMethodAndHttpStatusIsOk)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    // Given: DELETE method
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpDelete().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_DELETE, handler.getMethod());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnResponse_WhenGetMethodAndHttpStatusIsBadRequest)
{
    // Given: server return BadRequest.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::BadRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called and httpStatus is BadRequest.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, pResponse->getCode());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::BadRequestResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_CallsOnResponseAndReadResponseBody_WhenGetMethodAndHttpStatusIsOkAndSetNullToCallObject)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    // Given: create Call Object
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync and set NULL to Call.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);
    pCall = NULL;

    // onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());


    // Then: read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_CallsOnResponseAfterCalledInterceptor_WhenSetInterceptorAndHttpStatusIsOk)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: set CallInterceptor
    MockInterceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*pMockCallInterceptor, intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_GET, handler.getMethod());

    // read response body and close
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnFailureWithHttpTimeoutException_WhenRequestTimeoutOccurred)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: set TimeoutSec
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setTimeoutSec(TimeoutSec).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onFailure is called and what is HttpTimeoutException.
    EXPECT_TRUE(pCallback->waitCompletion());
    handler.set(); // resume handler
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpTimeoutException*> (pWhat.get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CallsOnFailureWithHttpExecutionException_WhenInvalidProxy)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // Given: set invalid proxy
    EasyHttp::Builder httpClientBuilder;
    Proxy::Ptr pProxy = new Proxy("proxy.host.co.jp", 12345);
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onFailure is called and what is HttpExecutionException.
    // Wait at most 10 minutes, since the time until getaddrinfo detects an error
    // depends on the environment.
    ASSERT_TRUE(pCallback->waitCompletion(10 * 60 * 1000));
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_ThrowsHttpIllegalStateException_WhenCalledAfterExecute)
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

    // Given: call execute method.
    Response::Ptr pResponse = pCall->execute();

    // When: executeAsync.
    // Then: throws HttpIllegalStateException
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    EASYHTTPCPP_EXPECT_THROW(pCall->executeAsync(pCallback), HttpIllegalStateException, 100701);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_ThrowsHttpIllegalStateException_WhenCalledAfterExecuteAsync)
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

    // Given: call executeAsync method.
    HttpTestResponseCallback::Ptr pCallback1 = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback1);
    EXPECT_TRUE(pCallback1->waitCompletion());
    EXPECT_TRUE(pCallback1->getWhat().isNull());

    // When: executeAsync.
    // Then: throws HttpIllegalStateException
    HttpTestResponseCallback::Ptr pCallback2 = new HttpTestResponseCallback();
    EASYHTTPCPP_EXPECT_THROW(pCall->executeAsync(pCallback2), HttpIllegalStateException, 100701);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_CallsOnFailureWithHttpExecutionException_WhenCancelWasAcceptedWhileWaitingForCallback)
{
    // Given: set for wait handler
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

    // When: executeAsync and cancel.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);
    ASSERT_TRUE(handler.waitForStart(TestFailureTimeout));
    pCall->cancel();
    handler.set(); // resume handler

    // Then: onFailure is called and what is HttpExecutionException and isCancelled is true.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
    EXPECT_TRUE(pCall->isCancelled());
}

TEST_F(CallExecuteAsyncIntegrationTest,
        readResponseBodyStream_ThrowsHttpExecutionException_WhenCalledCancelAfterOnResponse)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // When: call cancel after onResponse.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    EXPECT_TRUE(pCall->cancel());

    // Then: read throws HttpExecutionException.
    ResponseBodyStream::Ptr pResponseBodyStream = pResponse->getBody()->getByteStream();
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// executeAsync前のcancel
// onFailure に exception が渡される。
TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_CallsOnFailureWithHttpExecutionException_WhenAfterCallCancel)
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

    // When: call executeAsync
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onFailure is called and what is HttpExecutionException and isCancelled is true.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
    EXPECT_TRUE(pCall->isCancelled());
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_Waits_WhenFiveAsynchronousRequestsAreInProgress)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // Given: 5個の executeAsync 呼び出しを handler で停止し、非同期実行の thread 数を max にする。
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

    // When: sixth executeAsync.
    // Then: sixth executeAsync は即実行されない。
    std::string path6 = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, inAdvanceCallCount);
    HttpTestCommonRequestHandler::WaitRequestHandler handler6;
    testServer.getTestRequestHandlerFactory().addHandler(path6, &handler6);
    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, path6);
    Request::Builder requestBuilder6;
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).httpGet().build();
    Call::Ptr pCall6 = pHttpClient->newCall(pRequest6);
    HttpTestResponseCallback::Ptr pCallback6 = new HttpTestResponseCallback();
    pCall6->executeAsync(pCallback6);
    // 実行されない。1秒だけ待ちます。
    ASSERT_FALSE(handler6.waitForStart(1000L));

    // 5個の executeAsync の処理を再開する。
    for (int i = 0; i < inAdvanceCallCount; i++) {
        handlers[i]->set();
    }
    for (int i = 0; i < inAdvanceCallCount; i++) {
        callbacks[i]->waitCompletion();
        EXPECT_FALSE(callbacks[i]->getResponse().isNull());
        ASSERT_FALSE(callbacks[i]->getResponse()->getBody().isNull());
        std::string responseBody = callbacks[i]->getResponse()->getBody()->toString();
        EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
    }

    // 6個目の executeAsync が実行される。
    ASSERT_TRUE(handler6.waitForStart(TestFailureTimeout));
    handler6.set();
    EXPECT_TRUE(pCallback6->waitCompletion());
    EXPECT_FALSE(pCallback6->getResponse().isNull());
    ASSERT_FALSE(pCallback6->getResponse()->getBody().isNull());
    std::string responseBody6 = pCallback6->getResponse()->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody6);
}

TEST_F(CallExecuteAsyncIntegrationTest, execute_BeExecutedImmediately_WhenFiveAsynchronousRequestsAreInProgress)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // Given: 5個の executeAsync 呼び出しを handler で停止し、非同期実行の thread 数を max にする。
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

    // When: execute.
    // Then: execute は即実行される。
    std::string path6 = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, inAdvanceCallCount);
    HttpTestCommonRequestHandler::OkRequestHandler handler6;
    testServer.getTestRequestHandlerFactory().addHandler(path6, &handler6);
    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, path6);
    Request::Builder requestBuilder6;
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).httpGet().build();
    Call::Ptr pCall6 = pHttpClient->newCall(pRequest6);
    Response::Ptr pResponse6 = pCall6->execute();
    ASSERT_FALSE(pResponse6.isNull());
    ASSERT_FALSE(pResponse6->getBody().isNull());
    std::string responseBody6 = pResponse6->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody6);

    // 5個の executeAsync の処理を再開する。
    for (int i = 0; i < inAdvanceCallCount; i++) {
        handlers[i]->set();
    }
    for (int i = 0; i < inAdvanceCallCount; i++) {
        callbacks[i]->waitCompletion();
        EXPECT_FALSE(callbacks[i]->getResponse().isNull());
        ASSERT_FALSE(callbacks[i]->getResponse()->getBody().isNull());
        std::string responseBody = callbacks[i]->getResponse()->getBody()->toString();
        EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
    }
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsyncOfOtherEasyHttp_BeExecutedImmediately_WhenFiveAsynchronousRequestsAreInProgress)
{
    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.build();

    // Given: 5個の executeAsync 呼び出しを handler で停止し、非同期実行の thread 数を max にする。
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
        Call::Ptr pCall = pHttpClient1->newCall(pRequest);
        calls.push_back(pCall);

        HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
        callbacks.push_back(pCallback);

        pCall->executeAsync(pCallback);
        ASSERT_TRUE(pHandler->waitForStart(TestFailureTimeout));
    }

    // When: sixth executeAsync.
    // Then: sixth executeAsync は即実行される。
    EasyHttp::Builder httpClientBuilder6;
    EasyHttp::Ptr pHttpClient6 = httpClientBuilder6.build();
    std::string path6 = StringUtil::format("%s/%d", HttpTestConstants::DefaultPath, inAdvanceCallCount);
    HttpTestCommonRequestHandler::OkRequestHandler handler6;
    testServer.getTestRequestHandlerFactory().addHandler(path6, &handler6);
    std::string url6 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, path6);
    Request::Builder requestBuilder6;
    Request::Ptr pRequest6 = requestBuilder6.setUrl(url6).httpGet().build();
    Call::Ptr pCall6 = pHttpClient6->newCall(pRequest6);
    HttpTestResponseCallback::Ptr pCallback6 = new HttpTestResponseCallback();
    pCall6->executeAsync(pCallback6);
    EXPECT_TRUE(pCallback6->waitCompletion());
    ASSERT_FALSE(pCallback6->getResponse().isNull());
    ASSERT_FALSE(pCallback6->getResponse()->getBody().isNull());
    std::string responseBody6 = pCallback6->getResponse()->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody6);

    // 5個の executeAsync の処理を再開する。
    for (int i = 0; i < inAdvanceCallCount; i++) {
        handlers[i]->set();
    }
    for (int i = 0; i < inAdvanceCallCount; i++) {
        callbacks[i]->waitCompletion();
        ASSERT_FALSE(callbacks[i]->getResponse().isNull());
        ASSERT_FALSE(callbacks[i]->getResponse()->getBody().isNull());
        std::string responseBody = callbacks[i]->getResponse()->getBody()->toString();
        EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
    }
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CanReadResponseBodyInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: prepare callback to read response body.
    AnyExecutionInResponseCallback::Ptr pCallback = new ReadResponseBodyInOnResponseCallback();

    // When: executeAsync and wait callback.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onResponse is called and read response body in OnResponse.
    EXPECT_FALSE(pCallback->getResponse().isNull());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, pCallback->getFirstResponseBody());
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_CanCancelInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: prepare callback to cancel.
    AnyExecutionInResponseCallback::Ptr pCallback = new CancelInOnResponseCallback(pCall);

    // When: executeAsync and wait callback.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onResponse is called and cancel in OnResponse.
    EXPECT_FALSE(pCallback->getResponse().isNull());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    EXPECT_TRUE(pCall->isCancelled());
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteThrowsHttpIllegalStateExceptionInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: prepare callback to execute.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteInOnResponseCallback(pCall);

    // When: executeAsync.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onResponse is called and execute throws HttpIllegalStateException in OnResponse.
    EXPECT_FALSE(pCallback->getResponse().isNull());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    EXPECT_TRUE(dynamic_cast<HttpIllegalStateException*>(pCallback->getSecondWhat().get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteAsyncThrowsHttpIllegalStateExceptionInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // Given: prepare callback to executeAsync.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteAsyncInOnResponseCallback(pCall);

    // When: executeAsync and wait callback.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onResponse is called and executeAsync throws HttpIllegalStateException in OnResponse.
    EXPECT_FALSE(pCallback->getResponse().isNull());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    EXPECT_TRUE(dynamic_cast<HttpIllegalStateException*>(pCallback->getSecondWhat().get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteByOtherCallSucceedsInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).httpGet().build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // Given: prepare callback to executeAsync.
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).httpGet().build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteByOtherCallInOnResponseCallback(pCall2);

    // When: executeAsync and wait callback.
    pCall1->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());

    // Then: onResponse is called and execute by other Call in OnResponse succeeded.
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);

    // read second response body.
    Response::Ptr pResponse2 = pCallback->getSecondResponse();
    ASSERT_FALSE(pResponse2.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getBody().isNull());
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody2);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteAsyncByOtherCallSucceedsInOnResponse_WhenOnResponseIsCalled)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    std::string url = HttpTestConstants::DefaultTestUrl;

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).httpGet().build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // Given: prepare callback to executeAsync.
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).httpGet().build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteAsyncByOtherCallInOnResponseCallback(pCall2);

    // When: executeAsync and wait callback.
    pCall1->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());

    // Then: onResponse is called and executeAsync by other Call in OnResponse succeeded.
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    ASSERT_FALSE(pResponse->getBody().isNull());
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);

    // read second response body.
    Response::Ptr pResponse2 = pCallback->getSecondResponse();
    ASSERT_FALSE(pResponse2.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getBody().isNull());
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody2);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteThrowsHttpIllegalStateExceptionInOnFailure_WhenOnFailureIsCalled)
{
    // Given: prepare http client and callback to execute.
    // not start HttpTestServer to cause network error.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // prepare callback to execute in OnFailure.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteInOnFailureCallback(pCall);

    // When: executeAsync and wait callback.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onFailure is called and execute throws HttpIllegalStateException in OnFailure
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
    EXPECT_TRUE(dynamic_cast<HttpIllegalStateException*>(pCallback->getSecondWhat().get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest,
        executeAsync_ExecuteAsyncThrowsHttpIllegalStateExceptionInOnFailure_WhenOnFailureIsCalled)
{
    // Given: prepare http client and callback to execute.
    // not start HttpTestServer to cause network error.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // prepare callback to executeAsync in OnFailure.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteAsyncInOnFailureCallback(pCall);

    // When: executeAsync and wait callback.
    pCall->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());

    // Then: onFailure is called and executeAsync throws HttpIllegalStateException in OnFailure
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
    EXPECT_TRUE(dynamic_cast<HttpIllegalStateException*>(pCallback->getSecondWhat().get()) != NULL);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_ExecuteByOtherCallSucceedsInOnFailure_WhenOnFailureIsCalled)
{
    // Given: set handler that 1st request is TimeoutSec, 2nd request is success.
    //        prepare callback to executeAsync in OnFailure.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);
    HttpTestCommonRequestHandler::OkRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(TestPath2, &handler2);
    testServer.start(HttpTestConstants::DefaultPort);

    // 1st request is timeout.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setTimeoutSec(TimeoutSec).build();

    Request::Builder requestBuilder1;
    std::string url1 = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).httpGet().build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // 2nd request is success.
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, TestPath2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).httpGet().build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // prepare callback to execute in OnFailure.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteByOtherCallInOnFailureCallback(pCall2);

    // When: executeAsync and wait callback.
    pCall1->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());
    handler1.set(); // resume first handler

    // Then: onFailure is called and execute by other Call in OnFailure succeeded.
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpTimeoutException*> (pWhat.get()) != NULL);

    // read second response body.
    Response::Ptr pResponse2 = pCallback->getSecondResponse();
    ASSERT_FALSE(pResponse2.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getBody().isNull());
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody2);
}

TEST_F(CallExecuteAsyncIntegrationTest, executeAsync_ExecuteAsyncByOtherCallSucceedsInOnFailure_WhenOnFailureIsCalled)
{
    // Given: set handler that 1st request is TimeoutSec, 2nd request is success.
    //        prepare callback to executeAsync in OnFailure.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler1;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1);
    HttpTestCommonRequestHandler::OkRequestHandler handler2;
    testServer.getTestRequestHandlerFactory().addHandler(TestPath2, &handler2);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setTimeoutSec(TimeoutSec).build();

    // 1st request is timeout.
    Request::Builder requestBuilder1;
    std::string url1 = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).httpGet().build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // 2nd request is success.
    Request::Builder requestBuilder2;
    std::string url2 = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, TestPath2);
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).httpGet().build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // prepare callback to executeAsync in OnFailure.
    AnyExecutionInResponseCallback::Ptr pCallback = new ExecuteAsyncByOtherCallInOnFailureCallback(pCall2);

    // When: executeAsync and wait callback.
    pCall1->executeAsync(pCallback);
    EXPECT_TRUE(pCallback->waitCompletion());
    handler1.set(); // resume first handler

    // Then: onFailure is called and executeAsync by other Call in OnFailure succeeded.
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpTimeoutException*> (pWhat.get()) != NULL);

    // read second response body.
    Response::Ptr pResponse2 = pCallback->getSecondResponse();
    ASSERT_FALSE(pResponse2.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ASSERT_FALSE(pResponse2->getBody().isNull());
    std::string responseBody2 = pResponse2->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody2);
}

} /* namespace test */
} /* namespace easyhttpcpp */
