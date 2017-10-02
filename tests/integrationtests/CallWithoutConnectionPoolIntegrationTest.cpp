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

#include "ConnectionConfirmationInterceptor.h"
#include "ConnectionInternal.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "CallInternal.h"

using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const char* const HeaderConnection = "Connection";
static const char* const HeaderValueKeepAlive = "Keep-Alive";

class CallWithoutConnectionPoolIntegrationTest : public HttpIntegrationTestCase {
};

// temporary の ConnectionPool は共有されない
TEST_F(CallWithoutConnectionPoolIntegrationTest,
        execute_UsesDifferentConnection_WhenNotSpecifiedConnectionPoolAndCallExecuteTwoTimesBySameUrl)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    ConnectionConfirmationInterceptor* pConfirmationInterceptor = new ConnectionConfirmationInterceptor();
    Interceptor::Ptr pNetworkInterceptor = pConfirmationInterceptor;

    // Given: create EasyHttp without ConnectionPool and execute sendRequest once.
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pNetworkInterceptor).build();
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;

    // 1st request.

    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();

    // request succeeded and Connection::Keep-Alive in Header.
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    ASSERT_THAT(pResponse1->getHeaders(), testutil::containsInHeader(HeaderConnection, HeaderValueKeepAlive));

    ResponseBody::Ptr pResponseBody1 = pResponse1->getBody();

    // connection in temporary connection pool become idle by to read and to close response body.
    pResponseBody1->toString();


    Connection::Ptr pConnection1 = pConfirmationInterceptor->getConnection();
    ASSERT_EQ(ConnectionInternal::Idle, static_cast<ConnectionInternal*>(pConnection1.get())->getStatus());

    pConfirmationInterceptor->clearConnection();

    // When: execute GET method with same url.
    // 2nd request.
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);
    Response::Ptr pResponse2 = pCall2->execute();

    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // Then: be used different Connection.
    Connection::Ptr pConnection2 = pConfirmationInterceptor->getConnection();
    EXPECT_NE(pConnection1.get(), pConnection2.get());
}

} /* namespace test */
} /* namespace easyhttpcpp */
