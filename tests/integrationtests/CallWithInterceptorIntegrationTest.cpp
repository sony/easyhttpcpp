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
#include "Poco/String.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

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
#include "HttpTestServer.h"
#include "MockInterceptor.h"
#include "EasyHttpCppAssertions.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestBaseRequestHandler.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithInterceptorIntegrationTest";
static const char* const HeaderTest1 = "X-Test1";
static const char* const HeaderValueTest1 = "Value1";
static const char* const HeaderTest2 = "X-Test2";
static const char* const HeaderValueTest2 = "Value2";
static const char* const RedirectResponseBody = "redirect data 1";
static const char* const RedirectPath = "/redirect";
static const char* const HeaderLocation = "Location";

class CallWithInterceptorIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
};

namespace {

std::string getRedirectFirstUrl()
{
    return HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, RedirectPath, HttpTestConstants::DefaultQuery);
}

std::string getRedirectSecondUrl()
{
    return HttpTestConstants::DefaultTestUrlWithQuery;
}

bool isFirstReDirectUrl(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    std::string url = pRequest->getUrl();
    EASYHTTPCPP_LOG_D(Tag, "IsFirstReDirectUrl: url=%s", url.c_str());
    return (url == getRedirectFirstUrl());
}

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

Response::Ptr delegateAddHeaderIntercept(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    Request::Builder requestBuilder(pRequest);
    Request::Ptr pNewRequest = requestBuilder.setHeader(HeaderTest1, HeaderValueTest1).build();
    Response::Ptr pResponse = chain.proceed(pNewRequest);
    Response::Builder responseBuilder(pResponse);
    Response::Ptr pNewResponse = responseBuilder.setHeader(HeaderTest2, HeaderValueTest2).build();
    return pNewResponse;
}

Response::Ptr delegateProceedAndGetConnection(Interceptor::Chain& chain)
{
    Response::Ptr pResponse = chain.proceed(chain.getRequest());
    Connection::Ptr pConnection = chain.getConnection();
    if (pConnection.isNull()) {
        Response::Builder builder;
        return builder.setCode(-1).build();
    } else {
        return pResponse;
    }
}

Response::Ptr delegateProceedCheckGetRequest(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    if (pRequest.isNull()) {
        Response::Builder builder;
        return builder.setCode(-1).build();
    }
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    if (pRequest->getUrl() != url) {
        EASYHTTPCPP_LOG_E(Tag, "delegateProceedCheckGetRequest: url is wrong.[%s]",
                pRequest->getUrl().c_str());
        Response::Builder builder;
        return builder.setCode(-1).build();
    }

    return chain.proceed(pRequest);
}

Response::Ptr delegateCallInterceptorProceedCheckGetConnection(Interceptor::Chain& chain)
{
    Response::Ptr pResponse = chain.proceed(chain.getRequest());
    Connection::Ptr pConnection = chain.getConnection();
    if (!pConnection.isNull()) {
        Response::Builder builder;
        return builder.setCode(-1).build();
    } else {
        return pResponse;
    }
}

class RedirectRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.setContentLength(strlen(RedirectResponseBody));

        std::string url = getRedirectSecondUrl();
        response.set(HeaderLocation, url);

        std::ostream& ostr = response.send();
        ostr << RedirectResponseBody;
    }
};
    
} /* namespace */

// CallInterceptor を指定、execute を呼び出す
// CallInterceptor::intercept が呼び出される。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CallsCallInterceptor_WhenGetMethodAndAddCallInterceptor)
{
    // Given: add call interceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: CallInterceptor is called.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// Interceptor で Request、Responseをへんこう
// へんこうした Header でServer にRequest され、へんこうしたResponse が返る。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_SendsChangedHeaderByCallInterceptor_WhenGetMethodAndChangeRequestInInterceptor)
{
    // Given: add call interceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateAddHeaderIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: be changed request and response header.
    EXPECT_EQ(HeaderValueTest1, handler.getRequestHeaders().get(HeaderTest1, ""));
    EXPECT_EQ(HeaderValueTest2, pResponse->getHeaderValue(HeaderTest2, ""));
}

// CallInterceptor を複すう指定、execute を呼び出す。
// CallInterceptorを登ろくした順に、CallInterceptor::intercept が呼び出される
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CallsCallInterceptorInTheOrder_WhenGetMethodAndAddTwoCallInterceptor)
{
    // Given: set two NetworkInterceptors.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor1 = new MockInterceptor();
    Interceptor::Ptr pMockCallInterceptor2 = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor1.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor2.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor1).
            addInterceptor(pMockCallInterceptor2).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: CallInterceptor is called in the order in which they were registered.
    Response::Ptr pResponse = pCall->execute();

    // request result
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// CallInterceptor::intercept で、getRequest を呼び出す。
// Request が取得できる。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CanGetRequestInCallInterceptor_WhenAddCallInterceptor)
{
    // Given: add call interceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedCheckGetRequest));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: getRequest get Request in CallInterceptor
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// CallInterceptor::intercept で、getConnection を呼び出す。
// getConnection で、NULL が返る。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CanGetConnectionInCallInterceptor_WhenAddCallInterceptor)
{
    // Given: set CallInterceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateCallInterceptorProceedCheckGetConnection));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: getConnection is NULL in CallInterceptor
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// CallInterceptor::interceptで NULL を返す。
// NULL が返る
TEST_F(CallWithInterceptorIntegrationTest, execute_ReturnsNull_WhenCallInterceptorReturnsNull)
{
    // Given: set CallInterceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Return(Response::Ptr(NULL)));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute and interceptor return NULL
    Response::Ptr pResponse = pCall->execute();

    // Then: response is NULL
    EXPECT_TRUE(pResponse.isNull());
}

// CallInterceptor::interceptでexeption を throw する。
// exception が throw される。
TEST_F(CallWithInterceptorIntegrationTest, execute_ThrowsExecption_WhenCallInterceptorThrowsException)
{
    // Given: set CallInterceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Throw(HttpExecutionException("exception from interceptor")));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute and interceptor throws exception
    // Then: throws exception
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpExecutionException, 100702);
}

// redirect の execute をじっこう(redirect は全てnetwork access する)
// １回だけ CallInterceptor が呼び出される。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CallsCallInterceptorOneTime_WhenGetMethodAndHttpStatusCodeIsTemporaryRedirect)
{
    // Given: request GET Method to Redirect url
    HttpTestServer testServer;
    RedirectRequestHandler redirectFirstHandler;
    HttpTestCommonRequestHandler::OkRequestHandler redirectSecondHandler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &redirectFirstHandler);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &redirectSecondHandler);
    testServer.start(HttpTestConstants::DefaultPort);

    // first time is redirect, second time is not redirect.
    Interceptor::Ptr pMockCallInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockCallInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockCallInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: CallInterceptor call 1 time.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// NetworkInterceptor を指定、network access ありの時
// NetworkInterceptor::intercept が呼び出される。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CallsNetworkInterceptor_WhenGetMethodAndAddNetworkInterceptor)
{
    // Given: add network interceptor
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: NetworkInterceptor is called.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// Interceptor で Request、Responseをへんこう
// へんこうした Header でServer にRequest され、へんこうしたResponse が返る。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_SendsChangedHeaderByNetworkInterceptor_WhenGetMethodAndChangeRequestInInterceptor)
{
    // Given: add network interceptor

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateAddHeaderIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: be changed request and response header.
    EXPECT_EQ(HeaderValueTest1, handler.getRequestHeaders().get(HeaderTest1, ""));
    EXPECT_EQ(HeaderValueTest2, pResponse->getHeaderValue(HeaderTest2, ""));
}

// NetworkInterceptor を複すう指定、network access ありの時
// NetworkInterceptorを登ろくした順に、NetworkInterceptor::intercept が呼び出される
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CallsNetworkInterceptorInTheOrder_WhenGetMethodAndAddTwoNetworkInterceptor)
{
    // Given: set two NetworkInterceptors.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor1 = new MockInterceptor();
    Interceptor::Ptr pMockNetworkInterceptor2 = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor1.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor2.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor1).
            addNetworkInterceptor(pMockNetworkInterceptor2).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: NetworkInterceptor is called in the order in which they were registered.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// executeじっ行の後は、Connection を取得する。
// Connection が返る
TEST_F(CallWithInterceptorIntegrationTest, execute_CanGetConnectionInNetworkInterceptor_WhenAddNetworkInterceptor)
{
    // Given: add network interceptor

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedAndGetConnection));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: get connection in NetworkInterceptor
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// NetworkInterceptor::intercept で、getRequest を呼び出す。
// Request が取得できる。
TEST_F(CallWithInterceptorIntegrationTest,
        execute_CanGetRequestInNetworkInterceptor_WhenAddNetworkInterceptor)
{
    // Given: add NetworkInterceptor

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedCheckGetRequest));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: getRequest get Request in NetworkInterceptor
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
}

// NetworkInterceptor::interceptで NULL を返す。
// NULL が返る
TEST_F(CallWithInterceptorIntegrationTest, execute_ReturnsNull_WhenNetworkInterceptorReturnsNull)
{
    // Given: add NetworkInterceptor

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Return(Response::Ptr(NULL)));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute and interceptor return NULL
    Response::Ptr pResponse = pCall->execute();

    // Then: response is NULL
    EXPECT_TRUE(pResponse.isNull());
}

// NetworkInterceptor::interceptでexeption を throw する。
// exception が throw される。
TEST_F(CallWithInterceptorIntegrationTest, execute_ThrowsExecption_WhenNetworkInterceptorThrowsException)
{
    // Given: add NetworkInterceptor

    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Throw(HttpExecutionException("exception from interceptor")));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute and interceptor throws exception
    // Then: throws exception
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpExecutionException, 100702);
}

} /* namespace test */
} /* namespace easyhttpcpp */
