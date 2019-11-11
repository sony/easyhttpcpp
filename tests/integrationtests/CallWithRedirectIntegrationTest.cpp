/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CommonMacros.h"
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
#include "HttpsTestServer.h"
#include "MockInterceptor.h"
#include "TestLogger.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestCommonRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestResponseCallback.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::HttpsTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithRedirectIntegrationTest";
static const char* const RedirectPath = "/redirect";
static const char* const HeaderLocation = "Location";
static const char* const RedirectResponseBody = "redirect data 1";

class CallWithRedirectIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        Poco::Path certRootDir(HttpTestUtil::getDefaultCertRootDir());
        FileUtil::removeDirsIfPresent(certRootDir);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }
};

namespace {

std::string getRedirectFirstUrl()
{
    return HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, RedirectPath, HttpTestConstants::DefaultQuery);
}

std::string getRedirectFirstUrlWithoutQuery()
{
    return HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, RedirectPath);
}

std::string getRedirectSecondUrl()
{
    return HttpTestConstants::DefaultTestUrlWithQuery;
}

std::string getRedirectToHttpsUrl()
{
    return HttpTestUtil::makeUrl(HttpTestConstants::Https, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, RedirectPath, HttpTestConstants::DefaultQuery);
}

std::string getRedirectToHttpUrl()
{
    return HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
            HttpTestConstants::DefaultPort, RedirectPath, HttpTestConstants::DefaultQuery);
}

bool isFirstReDirectUrl(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    std::string url = pRequest->getUrl();
    EASYHTTPCPP_TESTLOG_I(Tag, "isFirstReDirectUrl: url=%s", url.c_str());
    return (url == getRedirectFirstUrl());
}

bool isFirstReDirectUrlWithoutQuery(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    std::string url = pRequest->getUrl();
    EASYHTTPCPP_TESTLOG_I(Tag, "isFirstReDirectUrlWithoutQuery: url=%s", url.c_str());
    return (url == getRedirectFirstUrlWithoutQuery());
}

bool isSecondReDirectUrl(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
    std::string url = pRequest->getUrl();
    EASYHTTPCPP_TESTLOG_I(Tag, "isSecondReDirectUrl: url=%s", url.c_str());
    return (url == getRedirectSecondUrl());
}

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

class RedirectRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.setContentLength(strlen(RedirectResponseBody));
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::string url = getRedirectSecondUrl();
        response.set(HeaderLocation, url);

        std::ostream& ostr = response.send();
        ostr << RedirectResponseBody;
    }
};

class SameRedirectRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.setContentLength(strlen(RedirectResponseBody));

        std::string url = getRedirectFirstUrl();
        response.set(HeaderLocation, url);

        std::ostream& ostr = response.send();
        ostr << RedirectResponseBody;
    }
};

class RedirectToHttpsRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.setContentLength(strlen(RedirectResponseBody));
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::string url = getRedirectToHttpsUrl();
        response.set(HeaderLocation, url);

        std::ostream& ostr = response.send();
        ostr << RedirectResponseBody;
    }
};

class RedirectToHttpRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.setContentLength(strlen(RedirectResponseBody));
        response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

        std::string url = getRedirectToHttpUrl();
        response.set(HeaderLocation, url);

        std::ostream& ostr = response.send();
        ostr << RedirectResponseBody;
    }
};

} /* namespace */

// 307:Temporary Redirect の場合、redirect を行う。
// retry する。
// PriorResponse にredirect 前のresponse が格納される。
TEST_F(CallWithRedirectIntegrationTest,
        execute_ReturnsRedirectedResponse_WhenGetMethodAndHttpStatusCodeIsTemporaryRedirect)
{
    // Given: set redirect handler (1st is redirect, 2nd is not redirect)
    HttpTestServer testServer;
    RedirectRequestHandler redirectFirstHandler;
    HttpTestCommonRequestHandler::OkRequestHandler redirectSecondHandler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &redirectFirstHandler);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &redirectSecondHandler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isSecondReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response after redirect
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    EXPECT_EQ(getRedirectSecondUrl(), pResponse->getRequest()->getUrl());
    Response::Ptr pPriorResponse = pResponse->getPriorResponse();
    ASSERT_FALSE(pPriorResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pPriorResponse->getCode());
    EXPECT_EQ(url, pPriorResponse->getRequest()->getUrl());
    EXPECT_TRUE(pPriorResponse->getBody().isNull());

    // check response body
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

// 6回 redirect する。
// 5回 retry し、HttpExecutionException がthrowされる。
TEST_F(CallWithRedirectIntegrationTest, execute_ThrowHttpExecutionException_WhenGetMethodAndRedirect6Times)
{
    // Given: setup infinity redirect.
    HttpTestServer testServer;
    SameRedirectRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).
            Times(6).WillRepeatedly(testing::Invoke(delegateProceedOnlyIntercept)); // retry 5 times is repeat 6 times.

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    // Then: throw exception after 5 times retry.
    EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpExecutionException, 100702);
}

// POST のredirect
// redirect しない。
TEST_F(CallWithRedirectIntegrationTest,
        execute_ReturnsResponseWithoutRedirect_WhenPostMethodAndHttpStatusCodeIsTemporaryRedirect)
{
    // Given: request POST Method to Redirect url
    HttpTestServer testServer;
    RedirectRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    // first time is redirect, second time is not redirect.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrlWithoutQuery))).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isSecondReDirectUrl))).Times(0);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrlWithoutQuery();
    Request::Ptr pRequest = requestBuilder.setUrl(url).httpPost().build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response not retry redirect.
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pResponse->getCode());
    EXPECT_EQ(url, pResponse->getRequest()->getUrl());
    Response::Ptr pPriorResponse = pResponse->getPriorResponse();
    ASSERT_TRUE(pPriorResponse.isNull());

    // check response body
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(RedirectResponseBody, responseBody);
}

// cache を使用している場合の、redirect
// redirect のresponse がcache に格納される。
TEST_F(CallWithRedirectIntegrationTest, execute_StoresRedirectResponseToCache_WhenUseCache)
{
    // Given: request GET Method to Redirect url
    HttpTestServer testServer;
    RedirectRequestHandler redirectFirstHandler;
    HttpTestCommonRequestHandler::OkRequestHandler redirectSecondHandler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &redirectFirstHandler);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &redirectSecondHandler);
    testServer.start(HttpTestConstants::DefaultPort);

    // first time is redirect, second time is not redirect.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    // first request for make cached response
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method (use cache).
    Response::Ptr pResponse = pCall->execute();

    // Then: receive after redirect response and store to cache.
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    EXPECT_EQ(getRedirectSecondUrl(), pResponse->getRequest()->getUrl());
    Response::Ptr pPriorResponse = pResponse->getPriorResponse();
    ASSERT_FALSE(pPriorResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pPriorResponse->getCode());
    EXPECT_EQ(url, pPriorResponse->getRequest()->getUrl());
    EXPECT_TRUE(pPriorResponse->getBody().isNull());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);

    // check redirect response in database
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(HttpTestUtil::createDatabasePath(cachePath)));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata1;
    std::string key1 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    pMetadata1 = db.getMetadataAll(key1);
    ASSERT_FALSE(pMetadata1.isNull());
    EXPECT_EQ(url, pMetadata1->getUrl());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pMetadata1->getStatusCode());
    EXPECT_EQ(strlen(RedirectResponseBody), pMetadata1->getResponseBodySize());

    // check cached response body
    size_t expectResponseBodySize1 = strlen(RedirectResponseBody);
    Poco::File responseBodyFile1(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_EQ(expectResponseBodySize1, responseBodyFile1.getSize());
    Poco::FileInputStream responseBodyStream1(responseBodyFile1.path(), std::ios::in | std::ios::binary);
    Poco::Buffer<char> inBuffer1(expectResponseBodySize1);
    responseBodyStream1.read(inBuffer1.begin(), expectResponseBodySize1);
    EXPECT_EQ(expectResponseBodySize1, responseBodyStream1.gcount());
    EXPECT_EQ(0, memcmp(inBuffer1.begin(), RedirectResponseBody, expectResponseBodySize1));

    // check user response in database
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata2;
    std::string url2 = getRedirectSecondUrl();
    std::string key2 = HttpUtil::makeCacheKey(Request::HttpMethodGet, url2);
    pMetadata2 = db.getMetadataAll(key2);
    ASSERT_FALSE(pMetadata2.isNull());
    EXPECT_EQ(url2, pMetadata2->getUrl());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pMetadata2->getStatusCode());
    EXPECT_EQ(strlen(HttpTestConstants::DefaultResponseBody), pMetadata2->getResponseBodySize());
    
    // check cached response body
    size_t expectResponseBodySize2 = strlen(HttpTestConstants::DefaultResponseBody);
    Poco::File responseBodyFile2(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url2));
    EXPECT_EQ(expectResponseBodySize2, responseBodyFile2.getSize());
    Poco::FileInputStream responseBodyStream2(responseBodyFile2.path(), std::ios::in | std::ios::binary);
    Poco::Buffer<char> inBuffer2(expectResponseBodySize2);
    responseBodyStream2.read(inBuffer2.begin(), expectResponseBodySize2);
    EXPECT_EQ(expectResponseBodySize2, responseBodyStream2.gcount());
    EXPECT_EQ(0, memcmp(inBuffer2.begin(), HttpTestConstants::DefaultResponseBody, expectResponseBodySize2));
}

// 307: TemporaryRedirect がCache に格納されている時の、redirect
// Cache の Response を使用して redirect される。
TEST_F(CallWithRedirectIntegrationTest,
        execute_ReturnsRedirectResponseFromCache_WhenExistsRedirectInCachedResponse)
{
    // Given: store 307 response to cache.
    HttpTestServer testServer;
    RedirectRequestHandler redirectFirstHandler;
    HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler redirectSecondHandler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &redirectFirstHandler);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &redirectSecondHandler);
    testServer.start(HttpTestConstants::DefaultPort);

    // first time is redirect, second time is not redirect third is not network access.
    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isSecondReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).Times(0);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor).
            build();

    // first request make cached response
    Request::Builder requestBuilder1;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    Call::Ptr pCall1 = pHttpClient->newCall(pRequest1);

    // execute GET method.
    Response::Ptr pResponse1 = pCall1->execute();

    // read response body and close
    std::string responseBody1 = pResponse1->getBody()->toString();

    // second request use cached response
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    Call::Ptr pCall2 = pHttpClient->newCall(pRequest2);

    // When: execute GET method (use cache).
    Response::Ptr pResponse2 = pCall2->execute();

    // Then: receive response after redirect with cached response
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse2->getCode());
    EXPECT_FALSE(pResponse2->getCacheResponse().isNull());
    EXPECT_EQ(getRedirectSecondUrl(), pResponse2->getRequest()->getUrl());
    Response::Ptr pPriorResponse2 = pResponse2->getPriorResponse();
    ASSERT_FALSE(pPriorResponse2.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pPriorResponse2->getCode());
    EXPECT_EQ(url, pPriorResponse2->getRequest()->getUrl());
    EXPECT_TRUE(pPriorResponse2->getBody().isNull());
    EXPECT_FALSE(pPriorResponse2->getCacheResponse().isNull());
}

// http から https への redirect
// redirect なしでresponse が返る。
TEST_F(CallWithRedirectIntegrationTest,
        execute_ReturnsResponseWithoutRedirect_WhenGetMethodAndHttpStatusCodeIsTemporaryRedirectAndRedirectToHttpsFromHttp)
{
    // Given: set http to https redirect handler
    HttpTestServer testServer;
    RedirectToHttpsRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::_)).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method.
    Response::Ptr pResponse = pCall->execute();

    // Then: receive response without redirect
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pResponse->getCode());
    EXPECT_EQ(url, pResponse->getRequest()->getUrl());
    Response::Ptr pPriorResponse = pResponse->getPriorResponse();
    EXPECT_TRUE(pPriorResponse.isNull());
    EXPECT_EQ(getRedirectToHttpsUrl(), pResponse->getHeaderValue(HeaderLocation, ""));

    // check response body
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(RedirectResponseBody, responseBody);
}

// executeAsync
// 307:Temporary Redirect の場合、redirect を行う。
// retry する。
// PriorResponse にredirect 前のresponse が格納される。
TEST_F(CallWithRedirectIntegrationTest,
        executeAsync_CallsOnResponseWithRedirectedResponse_WhenGetMethodAndHttpStatusCodeIsTemporaryRedirect)
{
    // Given: set redirect handler (1st is redirect, 2nd is not redirect)
    HttpTestServer testServer;
    RedirectRequestHandler redirectFirstHandler;
    HttpTestCommonRequestHandler::OkRequestHandler redirectSecondHandler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &redirectFirstHandler);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &redirectSecondHandler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isSecondReDirectUrl))).WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called after redirect.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getWhat().isNull());
    Response::Ptr pResponse = pCallback->getResponse();
    ASSERT_FALSE(pResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());
    Response::Ptr pPriorResponse = pResponse->getPriorResponse();
    ASSERT_FALSE(pPriorResponse.isNull());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT, pPriorResponse->getCode());
    EXPECT_EQ(url, pPriorResponse->getRequest()->getUrl());
    EXPECT_TRUE(pPriorResponse->getBody().isNull());

    // check response body
    std::string responseBody = pResponse->getBody()->toString();
    EXPECT_EQ(HttpTestConstants::DefaultResponseBody, responseBody);
}

// executeAsync
// 6回 redirect する。
// 5回 retry し、HttpExecutionException が OnFailire に渡される。
TEST_F(CallWithRedirectIntegrationTest, executeAsync_CallsOnFailureWithHttpExecutionException_WhenGetMethodAndRedirect6Times)
{
    // Given: setup infinity redirect.
    HttpTestServer testServer;
    SameRedirectRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(RedirectPath, &handler);
    testServer.start(HttpTestConstants::DefaultPort);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())),
            intercept(testing::Truly(isFirstReDirectUrl))).
            Times(6).WillRepeatedly(testing::Invoke(delegateProceedOnlyIntercept)); // retry 5 times is repeat 6 times.

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.addNetworkInterceptor(pMockNetworkInterceptor).build();
    Request::Builder requestBuilder;
    std::string url = getRedirectFirstUrl();
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync.
    HttpTestResponseCallback::Ptr pCallback = new HttpTestResponseCallback();
    pCall->executeAsync(pCallback);

    // Then: onResponse is called after redirect.
    EXPECT_TRUE(pCallback->waitCompletion());
    EXPECT_TRUE(pCallback->getResponse().isNull());
    HttpException::Ptr pWhat = pCallback->getWhat();
    ASSERT_FALSE(pWhat.isNull());
    EXPECT_TRUE(dynamic_cast<HttpExecutionException*> (pWhat.get()) != NULL);
}

} /* namespace test */
} /* namespace easyhttpcpp */
