/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/Buffer.h"
#include "Poco/Event.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "EasyHttpCppAssertions.h"
#include "HttpTestServer.h"
#include "TestDefs.h"
#include "TestPreferences.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestBaseRequestHandler.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::TestPreferences;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithGetMethodIntegrationTest";
static const char* const HeaderContentType = "Content-Type";
static const char* const InvalidQuery = "a==10";
static const char* const HeaderName1 = "name1";
static const char* const HeaderValue1 = "value1";
static const char* const HeaderName2 = "name2";
static const char* const HeaderValue2 = "value2";
static const char* const UseProxyUrlText = "http://d3d9bizloqaofq.cloudfront.net/1008/config/data4_1441001773.txt";
static const char* const UseProxyUrlTextData = EASYHTTPCPP_RAW({\n\t"hoge4" : "fuga4"\n}\n);
static const char* const UseProxyUrlBinary =
        "http://d3d9bizloqaofq.cloudfront.net/1008_BINARY/bin/data_5000_1441009016.bin";
static const char* const UnsupportedScheme = "ftp";
static const char* const InvalidHost = "//invalidhost";
static const int TimeoutSec = 1;
static const size_t ResponseBufferBytes = 8192;
static const size_t UseProxyUrlBinaryDataBytes = 5000;
static const unsigned short InvalidPort = 9000;
static const char* const PathContainsSpaceAndMultiByte = "/path/1 2/漢字";
static const char* const UrlThatPathContainsSpaceAndMultiByte = "http://localhost:9982/path/1 2/漢字?a=10&b=20";
static const char* const UrlThatFragmentContainsSpaceAndMultiByte = "http://localhost:9982/path?a=10&b=20#1 2漢字";

class CallWithGetMethodIntegrationTest : public HttpIntegrationTestCase {
public:
    static void SetUpTestCase()
    {
        // initialize test preferences with QA profile
        TestPreferences::getInstance().initialize(TestPreferences::ProfileQA);
    }

    static void TearDownTestCase()
    {
        std::cout << "fin" << std::endl;
    }
};

namespace {

class NoResponseContentLengthRequestHandler : public HttpTestBaseRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        saveRequestParamemter(request);

        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

} /* namespace */

TEST_F(CallWithGetMethodIntegrationTest,
        execute_ReturnsResponseAndCanReceiveResponseBody_WhenOmitMethod)
{
    // Given: request GET method to omit method function of Request::Builder.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    // not use httpGet function.
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: check request parameter and receive response from server
    // isSuccessFul and StatuCode == 200
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // request GET Method
    EXPECT_EQ(HttpTestConstants::DefaultPath, handler.getUri().getPath());
    EXPECT_EQ(HttpTestConstants::DefaultQuery, handler.getUri().getQuery());
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_GET, handler.getMethod());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body
    size_t expectResponseBodySize = strlen(HttpTestConstants::DefaultResponseBody);
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    ASSERT_EQ(expectResponseBodySize, pResponseBodyStream->read(responseBodyBuffer.begin(),
            ResponseBufferBytes));
    ASSERT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, expectResponseBodySize));
}

TEST_F(CallWithGetMethodIntegrationTest,
        execute_SendsRequestHeaderIsPocoDefaultAndReturnsResponse_WhenNoRequestHeader)
{
    // Given: request GET method to server with no request header
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: request header is Poco default setting ("Host") only.
    const Poco::Net::NameValueCollection& requestHeaders = handler.getRequestHeaders();
    EXPECT_EQ(1, requestHeaders.size());
    EXPECT_TRUE(requestHeaders.has("Host"));
}


TEST_F(CallWithGetMethodIntegrationTest, execute_ReturnsResponseAndReceivesBadRequest_WhenHttpStatusIsBadRequest)
{
    // Given: request GET method to return bad request server
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::BadRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive bad request and isSuccessful is false.
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, pResponse->getCode());
    EXPECT_FALSE(pResponse->isSuccessful());
}

TEST_F(CallWithGetMethodIntegrationTest, execute_SendsByNoQueryAndReturnsResponse_WhenNoQuery)
{
    // Given: request GET method to server without query
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: send without query
    EXPECT_TRUE(handler.getUri().getQuery().empty());
}

TEST_F(CallWithGetMethodIntegrationTest, execute_SendsInvalidQueryAndReturnsResponse_WhenInvalidQuery)
{
    // Given: request GET method to server with query format is invalid
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, InvalidQuery);
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: send invalud query
    EXPECT_EQ(InvalidQuery, handler.getUri().getQuery());
}

TEST_F(CallWithGetMethodIntegrationTest,
        execute_SendsSpecfiedRequestHeaderAndReturnsResponse_WhenSpecifyRequestHeader)
{
    // Given: request GET method to server with request header
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .setHeader(HeaderName1, HeaderValue1)
            .setHeader(HeaderName2, HeaderValue2)
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: send specified request header
    EXPECT_EQ(HeaderValue1, handler.getRequestHeaders().get(HeaderName1));
    EXPECT_EQ(HeaderValue2, handler.getRequestHeaders().get(HeaderName2));
}

TEST_F(CallWithGetMethodIntegrationTest,
        execute_ReturnsResponseAndReceivesResponseBody_WhenContentLengthNotExistInResponse)
{
    // Given: request GET method to server. server not set content-length.
    HttpTestServer testServer;
    NoResponseContentLengthRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    // create Request
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: receive response body without Content-Length
    EXPECT_FALSE(pResponse->hasContentLength());

    // read response body
    size_t expectResponseBodySize = strlen(HttpTestConstants::DefaultResponseBody);
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    ASSERT_EQ(expectResponseBodySize, pResponseBodyStream->read(responseBodyBuffer.begin(),
            ResponseBufferBytes));
    ASSERT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, expectResponseBodySize));
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpTimeoutException_WhenRequestTimeoutOccurred)
{
    // Given: request GET method to server. set timeout to EasyHttp.
    //        server request handler wait forever.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::WaitRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setTimeoutSec(TimeoutSec).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    Poco::Timestamp startTime;

    try {

        // When: execute GET method.
        Response::Ptr pResponse = pCall->execute();

        handler.set();  // resume handler
        FAIL() << "timeout did not occur.";

    } catch (const HttpTimeoutException& e) {
        handler.set();  // resume handler

        // Then: timeout occurred.
        Poco::Timestamp endTime;
        long long startRange = static_cast<long long>(TimeoutSec) * 1000000 - 500 * 1000; // - 500ms
        long long endRange = static_cast<long long>(TimeoutSec) * 1000000 + 500 * 1000;   // + 500ms
        // 500ms から 1500ms ならば OK
        EXPECT_THAT(endTime.epochMicroseconds() - startTime.epochMicroseconds(),
                testing::AllOf(testing::Ge(startRange), testing::Le(endRange)));
    } catch (const Poco::Exception& e) {
        handler.set();  // resume handler
        FAIL() << "other exception occurred.[" << e.message() << "]";
    }
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ReturnsResponseAndReceivesResponseBody_WhenGetResponseBodyAsString)
{
    // Given: request GET method to server.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: receive response body as string.
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    std::string resBody = pResponseBody->toString();
    ASSERT_EQ(HttpTestConstants::DefaultResponseBody, resBody);
}

TEST_F(CallWithGetMethodIntegrationTest, read_ThrowsHttpIllegalStateException_AfterCloseResponseBody)
{
    // Given: request GET method to server and close response body.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // close response body
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    pResponseBody->close();

    // When: read
    // Then: throw exception
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> buffer(ResponseBufferBytes);
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(buffer.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpIllegalStateException_WhenAlreadyExecute)
{
    // Given: request GET method to server and call Call:execute.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpGet()
            .build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // When: execute 2 times.
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpIllegalStateException, 100701);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ReturnResponseAndReceivesResponseBody_WhenUseProxyAndGetTextData)
{
    // Given: request GET method to external server with proxy.
    EasyHttp::Builder httpClientBuilder;
    Proxy::Ptr pProxy(TestPreferences::getInstance().optGetProxy(NULL));
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(UseProxyUrlText).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive succeeded and can receive response body.
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(UseProxyUrlTextData), contentLength);

    // read response body
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(UseProxyUrlTextData, responseBody);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ReturnsResponseAndReceivesResposeBody_WhenUseProxyAndGetBinaryData)
{
    // Given: request GET method to external server with proxy.
    EasyHttp::Builder httpClientBuilder;
    Proxy::Ptr pProxy(TestPreferences::getInstance().optGetProxy(NULL));
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(UseProxyUrlBinary).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // Then: read response body
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    Poco::Buffer<char> buffer(ResponseBufferBytes);
    ASSERT_EQ(UseProxyUrlBinaryDataBytes, HttpTestUtil::readAllData(pResponseBodyStream, buffer));

    // check response body
    for (size_t i = 0; i < UseProxyUrlBinaryDataBytes; i++) {
        ASSERT_EQ(0xff, (unsigned char) buffer[i]);
    }
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpExecutionException_WhenExternalServerAndInvalidProxySetting)
{
    // Given: request GET method to external server without proxy.
    Proxy::Ptr pProxy = new Proxy("proxy.host.co.jp", 12345);
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(UseProxyUrlText).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_UsesSameProxyAndReturnsResponse_WhenMultiCallsShareEasyHttp)
{
    // Given: request GET method to external server with proxy.
    EasyHttp::Builder httpClientBuilder;
    Proxy::Ptr pProxy(TestPreferences::getInstance().optGetProxy(NULL));
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setProxy(pProxy).build();

    // first call
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(UseProxyUrlText).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());

    // read response body
    std::string responseBody1 = pResponse1->getBody()->toString();
    ASSERT_EQ(UseProxyUrlTextData, responseBody1);

    // second call
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(UseProxyUrlBinary).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();

    // Then: receive succeeded.
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());

    // check response body
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    Poco::Buffer<char> buffer2(ResponseBufferBytes);
    ASSERT_EQ(UseProxyUrlBinaryDataBytes, HttpTestUtil::readAllData(pResponseBodyStream2, buffer2));

    // check response body
    for (size_t i = 0; i < UseProxyUrlBinaryDataBytes; i++) {
        ASSERT_EQ(0xff, (unsigned char) buffer2[i]);
    }
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpIllegalArgumentException_WhenUnsupportedScheme)
{
    // Given: unsupported scheme
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestUtil::makeUrl(UnsupportedScheme, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath, HttpTestConstants::DefaultQuery);

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpIllegalArgumentException, 100700);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpExecutionException_WhenInvallidHost)
{
    // Given: invalid host name
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, InvalidHost, HttpTestConstants::DefaultPort,
            HttpTestConstants::DefaultPath);

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
}

TEST_F(CallWithGetMethodIntegrationTest, execute_ThrowsHttpExecutionException_WhenInvalidPort)
{
    // Given: invalid port
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost, InvalidPort,
            HttpTestConstants::DefaultPath);

    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
}

TEST_F(CallWithGetMethodIntegrationTest,
        execute_ReturnsResponse_WhenPathContainsSpaceAndMultiByte)
{
    // Given: request GET method. path contains space and multi byte character.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(PathContainsSpaceAndMultiByte, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = UrlThatPathContainsSpaceAndMultiByte;
    // not use httpGet function.
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body
    size_t expectResponseBodySize = strlen(HttpTestConstants::DefaultResponseBody);
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    ASSERT_EQ(expectResponseBodySize, pResponseBodyStream->read(responseBodyBuffer.begin(),
            ResponseBufferBytes));
    ASSERT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, expectResponseBodySize));
}

TEST_F(CallWithGetMethodIntegrationTest,
        execute_ReturnsResponse_WhenFragmentContainsSpaceAndMultiByte)
{
    // Given: request GET method. fragment contains space and multi byte character.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = UrlThatFragmentContainsSpaceAndMultiByte;
    // not use httpGet function.
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body
    size_t expectResponseBodySize = strlen(HttpTestConstants::DefaultResponseBody);
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);
    ASSERT_EQ(expectResponseBodySize, pResponseBodyStream->read(responseBodyBuffer.begin(),
            ResponseBufferBytes));
    ASSERT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, expectResponseBodySize));
}

} /* namespace test */
} /* namespace easyhttpcpp */
