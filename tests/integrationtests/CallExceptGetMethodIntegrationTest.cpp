/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/Buffer.h"
#include "Poco/HashMap.h"
#include "Poco/MemoryStream.h"
#include "Poco/NumberFormatter.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "HttpTestServer.h"


#include "HttpIntegrationTestCase.h"
#include "HttpTestBaseRequestHandler.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallExceptGetMethodIntegrationTest";
static const char* const DefaultRequestContentType = "test/plain";
static const std::string DefaultRequestBody = "request body";
static const char* const HeaderContentType = "Content-Type";
static const char* const ContentTypeOctetStream = "application/octet-stream";
static const char* const HeaderContentLength = "Content-Length";
static const char* const ContentLength100 = "100";
static const size_t ResponseBufferBytes = 1024;
static const size_t RequestBodyLargeBytes = 1000000;
static const size_t RequestBodyBufferForLargeBytes = 2000000;

class CallExceptGetMethodIntegrationTest : public HttpIntegrationTestCase {
};

namespace {

class NoResponseBodyRequestHandler : public HttpTestBaseRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        saveRequestParameterAsString(request);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));

        response.send(); // no response body
    }
};

class OctetSteramRequestHandler : public HttpTestBaseRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        saveRequestParameterAsBinary(request);

        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));

        std::ostream& ostr = response.send();
        ostr << HttpTestConstants::DefaultResponseBody;
    }
};

} /* namespace */

TEST_F(CallExceptGetMethodIntegrationTest, execute_PostsDataToServerAndReturnsResponse_WhenHttpStatusIsOk)
{
    // Given: request POST method to server
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
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);

    requestBuilder.setUrl(url);
    requestBuilder.httpPost(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_POST, handler.getMethod());
    EXPECT_EQ(DefaultRequestContentType, handler.getRequestContentType());
    EXPECT_EQ(DefaultRequestBody.length(), handler.getRequestContentLength());
    EXPECT_EQ(DefaultRequestBody, handler.getRequestBody());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponseWithBadRequest_WhenHttpStatusIsBadRequest)
{
    // Given: request POST method to server
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::BadRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    RequestBody::Ptr pRequestBody;
    MediaType::Ptr pMediaType(new MediaType(DefaultRequestContentType));
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    requestBuilder.setUrl(url);
    requestBuilder.httpPost(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_FALSE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, pResponse->getCode());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::BadRequestResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest, execute_PutsDataToServerAndReturnsResponse_WhenHttpStatusIsOk)
{
    // Given: request PUT method to server
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
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);

    requestBuilder.setUrl(url);
    requestBuilder.httpPut(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_PUT, handler.getMethod());
    EXPECT_EQ(DefaultRequestContentType, handler.getRequestContentType());
    EXPECT_EQ(DefaultRequestBody.length(), handler.getRequestContentLength());
    EXPECT_EQ(DefaultRequestBody, handler.getRequestBody());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest, execute_PatchsDataToServerAndReturnsResponse_WhenHttpStatusIsOk)
{
    // Given: request PATCH method to server
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::BadRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    RequestBody::Ptr pRequestBody;
    MediaType::Ptr pMediaType(new MediaType(DefaultRequestContentType));
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    requestBuilder.setUrl(url);
    requestBuilder.httpPut(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_FALSE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_PUT, handler.getMethod());
    EXPECT_EQ(DefaultRequestContentType, handler.getRequestContentType());
    EXPECT_EQ(DefaultRequestBody.length(), handler.getRequestContentLength());
    EXPECT_EQ(DefaultRequestBody, handler.getRequestBody());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::BadRequestResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::BadRequestResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest, execute_ReturnsResponse_WhenDeleteMethodAndRequestBodyAndHttpStatusIsOk)
{
    // Given: request DELETE method to server
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
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);

    requestBuilder.setUrl(url);
    requestBuilder.httpDelete(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(HttpTestConstants::DefaultPath, handler.getUri().getPath());
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_DELETE, handler.getMethod());
    EXPECT_EQ(DefaultRequestContentType, handler.getRequestContentType());
    EXPECT_EQ(DefaultRequestBody.length(), handler.getRequestContentLength());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_ReturnsResponse_WhenDeleteMethodAndNoRequestBodyAndHttpStatusIsOk)
{
    // Given: request DELETE method with no request body to server
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    requestBuilder.httpDelete();
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(HttpTestConstants::DefaultPath, handler.getUri().getPath());
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_DELETE, handler.getMethod());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);
    EXPECT_EQ(HttpTestConstants::DefaultResponseContentType, pResponse->getHeaderValue(HeaderContentType, ""));

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenSmallRequestBody)
{
    // Given: request POST method to server with small request body.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = 20;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBufferStream);

    ssize_t requestContentLength = 20;
    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    requestBuilder.setHeader(HeaderContentLength, Poco::NumberFormatter::format(requestContentLength));
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(requestContentLength, handler.getRequestContentLength());
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenSmallRequestBodyAndRequestContentLengthIsNone)
{
    // Given: request POST method to server with small request body.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = 20;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBufferStream);

    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_POST, handler.getMethod());
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenBigRequestBody)
{
    // Given: request POST method to server with big request body.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    handler.setMaxRequestBodyBufferSize(RequestBodyBufferForLargeBytes);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = RequestBodyLargeBytes;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBufferStream);

    ssize_t requestContentLength = RequestBodyLargeBytes;
    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    requestBuilder.setHeader(HeaderContentLength, Poco::NumberFormatter::format(requestContentLength));
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(requestContentLength, handler.getRequestContentLength());
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenBigRequestBodyAndRequestContentLengthIsNone)
{
    // Given: request POST method to server with big request body.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    handler.setMaxRequestBodyBufferSize(RequestBodyBufferForLargeBytes);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = RequestBodyLargeBytes;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBufferStream);

    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenSmallRequestBodyByByteBuffer)
{
    // Given: request POST method to server with small request body with byte buffer.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = 20;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBuffer);

    ssize_t requestContentLength = 20;
    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(requestContentLength, handler.getRequestContentLength());
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_PostsDataToServerAndReturnsResponse_WhenBigRequestBodyByByteBuffer)
{
    // Given: request POST method to server with big request body with byte buffer.
    HttpTestServer testServer;
    OctetSteramRequestHandler handler;
    handler.setMaxRequestBodyBufferSize(RequestBodyBufferForLargeBytes);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;

    ssize_t requestDataBytes = RequestBodyLargeBytes;
    MediaType::Ptr pMediaType(new MediaType(ContentTypeOctetStream));
    ByteArrayBuffer requestBuffer(requestDataBytes);
    requestBuffer.setWrittenDataSize(requestDataBytes);
    Byte* pRequestBuffer = requestBuffer.getBuffer();
    for (ssize_t i = 0; i < requestDataBytes; i++) {
        pRequestBuffer[i] = (i & 0xff);
    }
    
    RequestBody::Ptr pRequestBody;
    Poco::MemoryInputStream requestBufferStream(reinterpret_cast<const char*>(pRequestBuffer),
            requestDataBytes);
    pRequestBody = RequestBody::create(pMediaType, requestBuffer);

    ssize_t requestContentLength = RequestBodyLargeBytes;
    std::string url = HttpTestConstants::DefaultTestUrl;
    requestBuilder.setUrl(url);
    Request::Ptr pRequest = requestBuilder.httpPost(pRequestBody).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute POST method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(requestContentLength, handler.getRequestContentLength());
    EXPECT_EQ(requestDataBytes, handler.getRequestBodyBytes());
    EXPECT_EQ(0, memcmp(pRequestBuffer, handler.getRequestBodyBuffer()->begin(), requestDataBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest, execute_SendsToServerByNoRequestBodyAndReturnsResponse_WhenHeadMethod)
{
    // Given: request HEAD method to server
    HttpTestServer testServer;
    NoResponseBodyRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpHead().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute HEAD method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response from server
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // check request
    EXPECT_EQ(Poco::Net::HTTPRequest::HTTP_HEAD, handler.getMethod());

    // check response header
    ssize_t contentLength = pResponse->getContentLength();
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody), contentLength);

    // check response body
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> buffer(ResponseBufferBytes);
    ASSERT_EQ(0, pResponseBodyStream->read(buffer.begin(), ResponseBufferBytes));
}

TEST_F(CallExceptGetMethodIntegrationTest,
        execute_SendsReallyContentLengthAndReturnsResponse_WhenSpecifiedContentLengthInRequestHeader)
{
    // Given: request POST method to server
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
    pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);

    requestBuilder.setUrl(url);
    requestBuilder.httpPost(pRequestBody).setHeader(HeaderContentLength, ContentLength100);
    Request::Ptr pRequest = requestBuilder.build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute.
    Response::Ptr pResponse = pCall->execute();
    EXPECT_TRUE(pResponse->isSuccessful());

    // Then: ContentLength replace by EasyHttp
    EXPECT_EQ(DefaultRequestContentType, handler.getRequestContentType());
    EXPECT_EQ(DefaultRequestBody.length(), handler.getRequestContentLength());
}

} /* namespace test */
} /* namespace easyhttpcpp */
