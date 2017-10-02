/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/NumberFormatter.h"
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
#include "EasyHttpCppAssertions.h"
#include "HttpTestServer.h"
#include "MockInterceptor.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "ResponseBodyStreamFromCacheIntegrationTest";

static const char* const DefaultRequestContentType = "text/plain";
static const std::string DefaultRequestBody = "request data 1";

static const size_t ResponseBufferBytes = 8192;

class ResponseBodyStreamFromCacheIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }
};

// Call::execute、Cache 使用で、ResponseBody を read
TEST_F(ResponseBodyStreamFromCacheIntegrationTest, read_ReadsData_WhenRequestSmallSizeThanResponseBodySize)
{
    // Given: create cache and next request use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // When: ResponesBodyStream::read by small size
    ssize_t readSize = 10;
    ssize_t retSize = pResponseBodyStream2->read(responseBodyBuffer2.begin(), readSize);

    // Then: read by requestedBytes
    EXPECT_GE(readSize, retSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer2.begin(), HttpTestConstants::DefaultResponseBody, retSize));
}

// Call::execute、Cache 使用で、ResponseBody サイズ以上を read
TEST_F(ResponseBodyStreamFromCacheIntegrationTest, read_ReadsData_WhenRequestLargeSizeThanResponseBodySize)
{
    // Given: create cache and next request use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ASSERT_FALSE(pResponseBody2.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // When: ResponesBodyStream::read by large size
    ssize_t retSize = pResponseBodyStream2->read(responseBodyBuffer2.begin(), ResponseBufferBytes);

    // Then: read by requestedBytes
    EXPECT_GE(strlen(HttpTestConstants::DefaultResponseBody), retSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer2.begin(), HttpTestConstants::DefaultResponseBody, retSize));
}

// read をれんぞくで呼び出す。
TEST_F(ResponseBodyStreamFromCacheIntegrationTest, read_ReadsContinuousData_WhenReadRepeatedlyInSmallSize)
{
    // Given: create cache and next request use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ASSERT_FALSE(pResponseBody2.isNull());
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // When: ResponesBodyStream::read repeatedly in small size
    ssize_t readSize = 10;
    ssize_t totalSize = 0;
    while(!pResponseBodyStream2->isEof()) {
        ssize_t retSize = pResponseBodyStream2->read(responseBodyBuffer2.begin() + totalSize, readSize);
        if (retSize > 0) {
            totalSize += retSize;
        }
    }

    // Then: read by all response body
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), totalSize);
    EXPECT_EQ(0, memcmp(responseBodyBuffer2.begin(), HttpTestConstants::DefaultResponseBody,
            strlen(HttpTestConstants::DefaultResponseBody)));
}

// Call::execute、Cache 使用で、ResponseBody::close を呼び出した後、read
TEST_F(ResponseBodyStreamFromCacheIntegrationTest, read_ThrowsHttpIllegalStateException_WhenAfterClose)
{
    // Given: create cache and next request use cache and next request execute until close.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // close
    pResponseBodyStream2->close();

    // When: ResponesBodyStream::read
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pResponseBodyStream2->read(responseBodyBuffer2.begin(), ResponseBufferBytes),
            HttpIllegalStateException, 100701);
}

// eof の後の、read
TEST_F(ResponseBodyStreamFromCacheIntegrationTest, read_ReturnsMinusOne_WhenAfterEof)
{
    // Given: create cache and next request use cache.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);

    // read response body until eof
    HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2);
    ASSERT_TRUE(pResponseBodyStream2->isEof());

    // When: ResponesBodyStream::read
    // Then: return -1
    EXPECT_EQ(-1, pResponseBodyStream2->read(responseBodyBuffer2.begin(), ResponseBufferBytes));
}

class MethodForRemoveBeforeCloseOfGetParam {
public:
    Request::HttpMethod httpMethod;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "httpMethod : " + StringUtil::format("%s", HttpUtil::httpMethodToString(httpMethod).c_str()) + "\n";
        return ret;
    }
};

static const MethodForRemoveBeforeCloseOfGetParam MethodForRemoveBeforeCloseOfGetData[] = {
    { // 0
        Request::HttpMethodPost // httpMethod
    },
    { // 1
        Request::HttpMethodPut // httpMethod
    },
    { // 2
        Request::HttpMethodDelete // httpMethod
    },
    { // 3
        Request::HttpMethodPatch // httpMethod
    }
};

class MethodForRemoveBeforeCloseOfGetTest : public ResponseBodyStreamFromCacheIntegrationTest,
        public testing::WithParamInterface<MethodForRemoveBeforeCloseOfGetParam> {
};
INSTANTIATE_TEST_CASE_P(ResponseBodyStreamFromCacheIntegrationTest, MethodForRemoveBeforeCloseOfGetTest,
        testing::ValuesIn(MethodForRemoveBeforeCloseOfGetData));

// 1. Call::execute、Cache 使用で、ResponseResponseBody2 は、ままだclose しない。
// 2. 同じurl をremove たいしょうの Method でCall::execute し、ResponseBody3 をclose
// 3. ここでは、Cache は削除されない
// 4. ResponseBody2 をclose すると Cache から、responseBody が削除される
TEST_P(MethodForRemoveBeforeCloseOfGetTest,
        close_RemovesCachedResponse_WhenAfterExecuteMethodForRemoveBeforeCloseResponseBodyOfGetMethod)
{
    MethodForRemoveBeforeCloseOfGetParam& param = (MethodForRemoveBeforeCloseOfGetParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given: create cache and next request use cache.
    //        when response body stream of next request not close, execute target method.
    HttpTestServer testServer;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder1;
    EasyHttp::Ptr pHttpClient1 = httpClientBuilder1.setCache(pCache).build();
    Request::Builder requestBuilder1;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient1->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse1->getCode());
    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // check cache
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    ASSERT_TRUE(db.getMetadataAll(key, metadata1));

    // GET same url from cached response.(not be called networkInterceptor)
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder2;
    EasyHttp::Ptr pHttpClient2 = httpClientBuilder2.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient2->newCall(pRequest2);

    // execute GET method and response body not close.
    Response::Ptr pResponse2 = pCall2->execute();
    ASSERT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    ResponseBody::Ptr pResponseBody2 = pResponse2->getBody();
    ResponseBodyStream::Ptr pResponseBodyStream2 = pResponseBody2->getByteStream();
    ASSERT_FALSE(pResponseBodyStream2.isNull());

    // execute target Method with same url
    EasyHttp::Builder httpClientBuilder3;
    EasyHttp::Ptr pHttpClient3 = httpClientBuilder3.setCache(pCache).build();
    Request::Builder requestBuilder3;
    switch (param.httpMethod) {
        case Request::HttpMethodDelete:
            requestBuilder3.httpDelete();
            break;
        case Request::HttpMethodPost:
            requestBuilder3.httpPost();
            break;
        case Request::HttpMethodPut:
        {
            MediaType::Ptr pMediaType = new MediaType(DefaultRequestContentType);
            RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
            requestBuilder3.httpPut(pRequestBody);
            break;
        }
        case Request::HttpMethodPatch:
        {
            MediaType::Ptr pMediaType = new MediaType(DefaultRequestContentType);
            RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
            requestBuilder3.httpPatch(pRequestBody);
            break;
        }
        default:
            break;
    }
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    Call::Ptr pCall3 = pHttpClient3->newCall(pRequest3);

    // execute POST method.
    Response::Ptr pResponse3 = pCall3->execute();
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse3->getCode());
    // read response body of target method and close
    std::string responseBody3 = pResponse3->getBody()->toString();

    // check cache
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    ASSERT_TRUE(db.getMetadataAll(key, metadata3));

    // read response body of Get
    Poco::Buffer<char> responseBodyBuffer2(ResponseBufferBytes);
    ASSERT_EQ(strlen(HttpTestConstants::DefaultResponseBody),
            HttpTestUtil::readAllData(pResponseBodyStream2, responseBodyBuffer2));

    // When: close
    pResponseBodyStream2->close();

    // Then: remove cached response
    HttpCacheDatabase::HttpCacheMetadataAll metadata4;
    EXPECT_FALSE(db.getMetadataAll(key, metadata4));
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_FALSE(responseBodyFile.exists());
}

} /* namespace test */
} /* namespace easyhttpcpp */
