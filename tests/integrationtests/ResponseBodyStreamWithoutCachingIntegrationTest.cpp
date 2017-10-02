/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/NumberFormatter.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Connection.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "EasyHttpCppAssertions.h"
#include "HttpTestServer.h"

#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "ResponseBodyStreamWithoutCaching.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "ResponseBodyStreamWithoutCachingIntegrationTest";
static const size_t ResponseBufferBytes = 8192;

class ResponseBodyStreamWithoutCachingIntegrationTest : public HttpIntegrationTestCase {
};

namespace {

class NoResponseBodyRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.send(); // no response body
    }
};

} /* namespace */

// Call::execute のあと、ResponseBody を read
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReadsData_WhenRequestSmallSizeThanResponseBodySize)
{
    // Given: not use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read by small size
    ssize_t readSize = 10;
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), readSize);

    // Then: read by requestedBytes
    EXPECT_GE(readSize, retSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, retSize));
}

// read をれんぞくで呼び出す。
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReadsContinuousData_WhenReadRepeatedlyInSmallSize)
{
    // Given: not use cache and read response by small size.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read repeatedly in small size
    ssize_t readSize = 10;
    ssize_t totalSize = 0;
    while(!pResponseBodyStream->isEof()) {
        ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin() + totalSize, readSize);
        if (retSize > 0) {
            totalSize += retSize;
        }
    }
    
    // Then: read by all response body
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), totalSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

// Call::execute のあと、ResponseBody サイズ以上を read
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReadsData_WhenRequestLargeSizeThanResponseBodySize)
{
    // Given: not use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read by large size
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);

    // Then: return read size
    EXPECT_GE(strlen(HttpTestConstants::DefaultResponseBody), retSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, retSize));
}

// Call::execute のあと、ResponseBody::close を呼び出し、read
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ThrowsHttpIllegalStateException_WhenAfterClose)
{
    // Given: not use cache and close response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());

    // close
    pResponseBodyStream->close();

    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read
    // Then: exception occurred.
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// read の pBuffer パラメータ == NULL
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ThrowsHttpIllegalArgumentException_WhenBufferIsNull)
{
    // Given: not use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());

    // When: ResponesBodyStream::read with pBuffer is NULL
    // Then: exception occurred.
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(NULL, ResponseBufferBytes),
            HttpIllegalArgumentException, 100700);
}

// read の readBytes パラメータ が、SSIZE_MAX 以上
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest,
        read_ThrowsHttpIllegalArgumentException_WhenReadBytesIsOverSSizeMax)
{
    // Given: not use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read with read size is SSIZE_MAX + 1
    // Then: exception occurred.
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->read(responseBodyBuffer.begin(), static_cast<size_t>(SSIZE_MAX) + 1),
            HttpIllegalArgumentException, 100700);
}

// Call を破棄した後に read
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReadsData_WhenAfterReleaseCall)
{
    // Given: not use cache and delete Call.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // release Call
    ASSERT_EQ(1, pCall->referenceCount());
    pCall = NULL;

    // When: ResponesBodyStream::read
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);

    // Then: read all data
    EXPECT_GE(strlen(HttpTestConstants::DefaultResponseBody), retSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer.begin(), HttpTestConstants::DefaultResponseBody, retSize));
}

// ResponseBody なしの場合のread
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReadsNoData_WhenNoResponseBody)
{
    // Given: not use cache and no response from server.
    HttpTestServer testServer;
    NoResponseBodyRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ASSERT_FALSE(pResponseBody.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // When: ResponesBodyStream::read
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes);

    // Then: read size = 0
    EXPECT_EQ(0, retSize);
}

// eof の後の、read
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, read_ReturnsMinusOne_WhenAfterEof)
{
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // read response body until eof
    HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    ASSERT_TRUE(pResponseBodyStream->isEof());

    // When: ResponesBodyStream::read
    // Then: return -1
    EXPECT_EQ(-1, pResponseBodyStream->read(responseBodyBuffer.begin(), ResponseBufferBytes));
}

// Call::execute のあと、ResponseBody サイズ以上を read し、isEof 呼び出し
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, isEof_ReturnsTrue_WhenAfterReadEndOfData)
{
    // Given: read all response body stream
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // read all data
    size_t retSize = HttpTestUtil::readAllData(pResponseBodyStream, responseBodyBuffer);
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), retSize);

    // When: isEof
    // Then: return true.
    EXPECT_TRUE(pResponseBodyStream->isEof());
}

// Call::execute のあと、isEof 呼び出し
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, isEof_ReturnsFalse_WhenBeforeRead)
{
    // Given: before read response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());

    // When: isEof
    // Then: return false.
    EXPECT_FALSE(pResponseBodyStream->isEof());
}

// Get Method でCall::execute のあと、ResponseBody よみこみ途中で isEof 呼び出し
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, isEof_ReturnsFalse_WhenReading)
{
    // Given: reading response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    Poco::Buffer<char> responseBodyBuffer(ResponseBufferBytes);

    // ResponesBodyStream::read by small size
    ssize_t readSize = 10;
    ssize_t retSize = pResponseBodyStream->read(responseBodyBuffer.begin(), readSize);
    EXPECT_GE(readSize, retSize);

    // When: isEof
    // Then: return false.
    EXPECT_FALSE(pResponseBodyStream->isEof());
}

// Call::execute のあと、ResponseBody::close を呼び出した後、isEof 呼び出し
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, isEof_ThrowsHttpIllegalStateException_WhenAfterClose)
{
    // Given: close response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());

    // close
    pResponseBodyStream->close();

    // When: isEof
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream->isEof(), HttpIllegalStateException, 100701);
}

// Call::execute のあと、ResponseBody::close を２回呼び出す
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest, close_NotThrowsException_WhenAfterClose)
{
    // Given:close response body stream.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    Response::Ptr pResponse = pCall->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());

    // close
    pResponseBodyStream->close();

    // When: close
    // Then: not throw exception
    pResponseBodyStream->close();
}

// close を呼び出すと、Connection のreference count が decrement される。
TEST_F(ResponseBodyStreamWithoutCachingIntegrationTest,
        close_DecrementsConnectionReferenceCount_WhenHasConnection)
{
    // Given: read response body stream
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OkRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method
    Response::Ptr pResponse = pCall->execute();
    ASSERT_TRUE(pResponse->isSuccessful());

    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    ASSERT_FALSE(pResponseBodyStream.isNull());
    ResponseBodyStreamWithoutCaching* pResponseBodyStreamWithoutCaching =
            static_cast<ResponseBodyStreamWithoutCaching*>(pResponseBodyStream.get());
    Connection::Ptr pConnection = pResponseBodyStreamWithoutCaching->getConnection();
    int beforeReferenceCount = pConnection->referenceCount();
    EASYHTTPCPP_LOG_D(Tag, "pConnection.referenceCount() = %d", beforeReferenceCount);

    // When: close response body stream with skip response body
    pResponseBodyStream->close();

    // Then: reference count was decremented 2 times. (ResponseBodyStream and HttpEngine)
    EXPECT_EQ(beforeReferenceCount - 2, pConnection->referenceCount());
}


} /* namespace test */
} /* namespace easyhttpcpp */
