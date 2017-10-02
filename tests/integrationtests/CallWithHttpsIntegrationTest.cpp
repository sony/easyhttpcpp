/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Buffer.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "EasyHttpCppAssertions.h"
#include "HttpsTestServer.h"
#include "MockInterceptor.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpsTestServer;
using easyhttpcpp::testutil::MockInterceptor;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithHttpsIntegrationTest";
static const char* const DefaultRequestContentType = "text/plain";
static const std::string DefaultRequestBody = "request data 1";
static const char* const DefaultResponseContentType = "text/plain";
static const char* const DefaultResponseBody = "response data 1";

static const char* const TestDataForValidCert = "/HttpIntegrationTest/02_https_localhost_cert/cert/";
static const char* const ServerCertFile = "server/server.pem";
static const char* const ValidRootCaDir = "client/rootCa/";
static const char* const ValidRootCaFile = "client/localhost_rootCa.pem";
static const char* const InvalidRootCaDir = "client/rootCa_invalid/";
static const char* const EmptyRootCaDir = "client/rootCa_empty/";
static const char* const InvalidRootCaFile = "client/invalid_rootCa.txt";
static const char* const TestDataForExpiredCert = "/HttpIntegrationTest/03_https_localhost_cert_expired/cert/";
static const char* const InvalidCommonNameRootCaDir = "client/rootCa-invalid-common/";
static const char* const InvalidCommonNameRootCaFile = "client/localhost_rootCa_invalid_common.pem";

class CallWithHttpsIntegrationTest : public HttpIntegrationTestCase {
protected:

   void SetUp()
    {
        Poco::Path cachePath(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(cachePath);

        Poco::Path certRootDir(HttpTestUtil::getDefaultCertRootDir());
        FileUtil::removeDirsIfPresent(certRootDir);
    }
};

namespace {

Response::Ptr delegateProceedOnlyIntercept(Interceptor::Chain& chain)
{
    return chain.proceed(chain.getRequest());
}

class DefaultRequestHandler : public Poco::Net::HTTPRequestHandler {
public:

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        response.setContentType(DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(strlen(DefaultResponseBody));

        std::ostream& ostr = response.send();
        if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_HEAD) {
            ostr << DefaultResponseBody;
        }
    }
};

} /* namespace */

class HttpsRootCaTestParam {
public:
    Request::HttpMethod httpMethod;
    const char* pCaLocation;
    const char* pCaFile;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "httpMethod : " + StringUtil::format("%s", HttpUtil::httpMethodToString(httpMethod).c_str()) + "\n" +
                "pCaLocation : " + (pCaLocation ? pCaLocation : "") + "\n" +
                "pCaFile : " + (pCaFile ? pCaFile : "") + "\n";
        return ret;
    }
};

static const HttpsRootCaTestParam HttpsValidRootCaTestData[] = {
    {   // GET, use rootCa directory
        Request::HttpMethodGet, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // GET, use rootCa file
        Request::HttpMethodGet, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // GET, use rootCa directory
        Request::HttpMethodGet, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile // pCaFile
    },
    {   // POST, use rootCa directory
        Request::HttpMethodPost, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // POST, use rootCa file
        Request::HttpMethodPost, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // PUT, use rootCa directory
        Request::HttpMethodPut, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // POST, use rootCa directory
        Request::HttpMethodPost, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // PUT, use rootCa file
        Request::HttpMethodPut, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // POST, use rootCa directory
        Request::HttpMethodPost, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // PATCH, use rootCa directory
        Request::HttpMethodPatch, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // PATCH, use rootCa file
        Request::HttpMethodPatch, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // PATCH, use rootCa directory
        Request::HttpMethodPatch, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // DELETE, use rootCa directory
        Request::HttpMethodDelete, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // DELETE, use rootCa file
        Request::HttpMethodDelete, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // DELETE, use rootCa directory
        Request::HttpMethodDelete, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // HEAD, use rootCa directory
        Request::HttpMethodHead, // httpMethod
        ValidRootCaDir, // pCaLocation
        NULL              // pCaFile
    },
    {   // HEAD, use rootCa file
        Request::HttpMethodHead, // httpMethod
        NULL,               // pCaLocation
        ValidRootCaFile   // pCaFile
    },
    {   // HEAD, use rootCa directory
        Request::HttpMethodHead, // httpMethod
        ValidRootCaDir, // pCaLocation
        ValidRootCaFile   // pCaFile
    },
};

class HttpsValidRootCaTest : public CallWithHttpsIntegrationTest,
        public testing::WithParamInterface<HttpsRootCaTestParam> {
};
INSTANTIATE_TEST_CASE_P(CallWithHttpsIntegrationTest, HttpsValidRootCaTest,
        testing::ValuesIn(HttpsValidRootCaTestData));

TEST_P(HttpsValidRootCaTest, execute_DoesNotOccurSslErrorAndReturnsResponse_WhenCombinationOfMethodAndRootCa)
{
    HttpsRootCaTestParam& param = (HttpsRootCaTestParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // load cert data
    std::string certRootParentDir = HttpTestUtil::getDefaultCertRootParentDir();
    Poco::File file(certRootParentDir);
    file.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForValidCert);
    srcTestData.copyTo(certRootParentDir);

    // set test handler
    HttpsTestServer testServer;
    DefaultRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // set cert and start server
    std::string certRootDir = HttpTestUtil::getDefaultCertRootDir();
    testServer.setCertUnitedFile(certRootDir + ServerCertFile);
    testServer.start(HttpTestConstants::DefaultHttpsPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // Given: set rootCa location and file.

    // create EasyHttp with rootCa
    EasyHttp::Builder httpClientBuilder;
    httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor);
    if (param.pCaLocation != NULL) {
        httpClientBuilder.setRootCaDirectory(certRootDir + param.pCaLocation);
    }
    if (param.pCaFile != NULL) {
        httpClientBuilder.setRootCaFile(certRootDir + param.pCaFile);
    }
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    Request::Builder requestBuilder;
    switch (param.httpMethod) {
        case Request::HttpMethodDelete:
            requestBuilder.httpDelete();
            break;
        case Request::HttpMethodPost:
            requestBuilder.httpPost();
            break;
        case Request::HttpMethodHead:
            requestBuilder.httpHead();
            break;
        case Request::HttpMethodPut:
        {
            MediaType::Ptr pMediaType = new MediaType(DefaultRequestContentType);
            RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
            requestBuilder.httpPut(pRequestBody);
            break;
        }
        default:
            requestBuilder.httpGet();
            break;
    }
    std::string url = HttpTestConstants::DefaultHttpsTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute GET method and ResponseBodyStream::close
    Response::Ptr pResponse = pCall->execute();

    // Then: request succeeded.
    EXPECT_TRUE(pResponse->isSuccessful());
    EXPECT_EQ(Poco::Net::HTTPResponse::HTTP_OK, pResponse->getCode());

    // read response body and close
    if (param.httpMethod != Request::HttpMethodHead) {
        std::string responseBody = pResponse->getBody()->toString();
    }

    // check database
    if (param.httpMethod == Request::HttpMethodGet) {
        HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
        HttpCacheDatabase::HttpCacheMetadataAll metadata;
        std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
        EXPECT_TRUE(db.getMetadataAll(key, metadata));
    }
}

class HttpsInvalidRootCaTestParam {
public:
    Request::HttpMethod httpMethod;
    const char* pCaLocation;
    const char* pCaFile;
    bool isSslException;
    bool withCause;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "httpMethod : " + StringUtil::format("%s", HttpUtil::httpMethodToString(httpMethod).c_str()) + "\n" +
                "pCaLocation : " + (pCaLocation ? pCaLocation : "") + "\n" +
                "pCaFile : " + (pCaFile ? pCaFile : "") + "\n" +
                "isSslException : " + StringUtil::boolToString(isSslException) + "\n" +
                "withCause : " + StringUtil::boolToString(withCause) + "\n";
        return ret;
    }
};

static const HttpsInvalidRootCaTestParam HttpsInvalidRootCaTestData[] = {
    {   // 0: GET, not set rootCa
        Request::HttpMethodGet, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 1: GET, invalid rootCa directory
        Request::HttpMethodGet, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false, // isSslException
        true // withCause
    },
    {   // 2: GET, empty rootCa directory
        Request::HttpMethodGet, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 3: GET, invalid rootCa file
        Request::HttpMethodGet, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 4: GET, invalid common name rootCa directory
        Request::HttpMethodGet, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 5: GET, invalid common name rootCa file
        Request::HttpMethodGet, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 6: POST, not set rootCa
        Request::HttpMethodPost, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 7: POST, invalid rootCa directory
        Request::HttpMethodPost, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false // isSslException
    },
    {   // 8: POST, empty rootCa directory
        Request::HttpMethodPost, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 9: POST, invalid rootCa file
        Request::HttpMethodPost, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 10: POST, invalid common name rootCa directory
        Request::HttpMethodPost, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 11: POST, invalid common name rootCa file
        Request::HttpMethodPost, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 12: PUT, not set rootCa
        Request::HttpMethodPut, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 13: PUT, invalid rootCa directory
        Request::HttpMethodPut, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false, // isSslException
        true // withCause
    },
    {   // 14: PUT, empty rootCa directory
        Request::HttpMethodPut, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 15: PUT, invalid rootCa file
        Request::HttpMethodPut, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 16: PUT, invalid common name rootCa directory
        Request::HttpMethodPut, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 17: PUT, invalid common name rootCa file
        Request::HttpMethodPut, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 18: PATCH, not set rootCa
        Request::HttpMethodPatch, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 19: PATCH, invalid rootCa directory
        Request::HttpMethodPatch, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false, // isSslException
        true // withCause
    },
    {   // 20: PATCH, empty rootCa directory
        Request::HttpMethodPatch, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 21: PATCH, invalid rootCa file
        Request::HttpMethodPatch, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 22: PATCH, invalid common name rootCa directory
        Request::HttpMethodPatch, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 23: PATCH, invalid common name rootCa file
        Request::HttpMethodPatch, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 24: DELETE, not set rootCa
        Request::HttpMethodDelete, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 25: DELETE, invalid rootCa directory
        Request::HttpMethodDelete, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false, // isSslException
        true // withCause
    },
    {   // 26: DELETE, empty rootCa directory
        Request::HttpMethodDelete, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 27: DELETE, invalid rootCa file
        Request::HttpMethodDelete, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 28: DELETE, invalid common name rootCa directory
        Request::HttpMethodDelete, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 29: DELETE, invalid common name rootCa file
        Request::HttpMethodDelete, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 30: HEAD, not set rootCa
        Request::HttpMethodHead, // httpMethod
        NULL, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 31: HEAD, invalid rootCa directory
        Request::HttpMethodHead, // httpMethod
        InvalidRootCaDir, // pCaLocation
        NULL, // pCaFile
        false, // isSslException
        true // withCause
    },
    {   // 32: HEAD, empty rootCa directory
        Request::HttpMethodHead, // httpMethod
        EmptyRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 33: HEAD, invalid rootCa file
        Request::HttpMethodHead, // httpMethod
        NULL, // pCaLocation
        InvalidRootCaFile, // pCaFile
        true, // isSslException
        false // withCause
    },
    {   // 34: HEAD, invalid common name rootCa directory
        Request::HttpMethodHead, // httpMethod
        InvalidCommonNameRootCaDir, // pCaLocation
        NULL, // pCaFile
        true, // isSslException
        true // withCause
    },
    {   // 35: HEAD, invalid common name rootCa file
        Request::HttpMethodHead, // httpMethod
        NULL, // pCaLocation
        InvalidCommonNameRootCaFile, // pCaFile
        true, // isSslException
        true // withCause
    },
};

class HttpsInvalidRootCaTest : public CallWithHttpsIntegrationTest,
        public testing::WithParamInterface<HttpsInvalidRootCaTestParam> {
};
INSTANTIATE_TEST_CASE_P(CallWithHttpsIntegrationTest, HttpsInvalidRootCaTest,
        testing::ValuesIn(HttpsInvalidRootCaTestData));

TEST_P(HttpsInvalidRootCaTest, execute_ThrowsException_WhenCombinationMethodAndRootCaInvalidCondition)
{
    HttpsInvalidRootCaTestParam& param = (HttpsInvalidRootCaTestParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // load cert data
    std::string certRootParentDir = HttpTestUtil::getDefaultCertRootParentDir();
    Poco::File file(certRootParentDir);
    file.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForValidCert);
    srcTestData.copyTo(certRootParentDir);

    // set test handler
    HttpsTestServer testServer;
    DefaultRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // set cert and start server
    std::string certRootDir = HttpTestUtil::getDefaultCertRootDir();
    testServer.setCertUnitedFile(certRootDir + ServerCertFile);
    testServer.start(HttpTestConstants::DefaultHttpsPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // Given: set invalid rootCa location or file

    // create EasyHttp without set rootCa directory
    EasyHttp::Builder httpClientBuilder;
    httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor);
    if (param.pCaLocation != NULL) {
        httpClientBuilder.setRootCaDirectory(certRootDir + param.pCaLocation);
    }
    if (param.pCaFile != NULL) {
        httpClientBuilder.setRootCaFile(certRootDir + param.pCaFile);
    }
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    Request::Builder requestBuilder;
    switch (param.httpMethod) {
        case Request::HttpMethodDelete:
            requestBuilder.httpDelete();
            break;
        case Request::HttpMethodPost:
            requestBuilder.httpPost();
            break;
        case Request::HttpMethodHead:
            requestBuilder.httpHead();
            break;
        case Request::HttpMethodPut:
        {
            MediaType::Ptr pMediaType = new MediaType(DefaultRequestContentType);
            RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
            requestBuilder.httpPut(pRequestBody);
            break;
        }
        default:
            requestBuilder.httpGet();
            break;
    }
    std::string url = HttpTestConstants::DefaultHttpsTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    if (param.isSslException) {
        if (param.withCause) {
            EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpSslException, 100704);
        } else {
            EASYHTTPCPP_EXPECT_THROW(pCall->execute(), HttpSslException, 100704);
        }
    } else {
        EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpExecutionException, 100702);
    }
}

TEST_F(CallWithHttpsIntegrationTest, execute_ThrowsHttpSslException_WhenServerCrlIsExpired)
{
    // load test data
    std::string certRootParentDir = HttpTestUtil::getDefaultCertRootParentDir();
    Poco::File file(certRootParentDir);
    file.createDirectories();
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForExpiredCert);
    srcTestData.copyTo(certRootParentDir);

    // set test handler
    HttpsTestServer testServer;
    DefaultRequestHandler handler;
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler);

    // Given: set expired cert to server and start server
    std::string certRootDir = HttpTestUtil::getDefaultCertRootDir();
    testServer.setCertUnitedFile(certRootDir + ServerCertFile);
    testServer.start(HttpTestConstants::DefaultHttpsPort);

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(Poco::Path(cachePath), HttpTestConstants::DefaultCacheMaxSize);

    Interceptor::Ptr pMockNetworkInterceptor = new MockInterceptor();
    EXPECT_CALL(*(static_cast<MockInterceptor*> (pMockNetworkInterceptor.get())), intercept(testing::_)).
            WillOnce(testing::Invoke(delegateProceedOnlyIntercept));

    // create EasyHttp
    EasyHttp::Builder httpClientBuilder;
    httpClientBuilder.setCache(pCache).addNetworkInterceptor(pMockNetworkInterceptor);
    httpClientBuilder.setRootCaDirectory(certRootDir + ValidRootCaDir);
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();

    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultHttpsTestUrl;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: execute
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCall->execute(), HttpSslException, 100704);
}

} /* namespace test */
} /* namespace easyhttpcpp */
