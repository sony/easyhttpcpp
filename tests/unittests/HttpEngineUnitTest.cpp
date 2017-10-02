/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "HttpEngine.h"
#include "HttpUtil.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const char* const DefaultRequestBody = "test request body";
static const char* const DefaultContentType = "text/plain";
static const char* const HeaderLocation = "Location";
static const char* const HeaderAuthorization = "Authorization";
static const char* const AuthorizationValue = "auth-01";

static const char* const RequestUrlHttp = "http://quiver.com/test1";
static const char* const RequestUrlHttps = "https://quiver.com/test1";
static const char* const RequestUrlHttpPort = "http://quiver.com:9000/test1";
static const char* const LocationUrlHttp = "http://quiver.com/redirect1";
static const char* const LocationUrlHttps = "https://quiver.com/redirect1";
static const char* const LocationUrlEncode = "http%3a%2f%2fquiver%2ecom%2fredirect1";
static const char* const LocationUrlHttpHost = "http://localhost/redirect1";
static const char* const LocationUrlHttpPort = "http://quiver.com:9001/redirect1";

class HttpEngineUnitTest : public testing::Test {
};

TEST(HttpEngineUnitTest, getRetryRequest_ReturnsNull_WhenResponseIsNull)
{
    // Given:
    HttpEngine httpEngine(NULL, NULL, NULL);

    // When: getRetryRequest
    Request::Ptr pRetryRequest = httpEngine.getRetryRequest(NULL);

    // Then: return NULL.
    ASSERT_TRUE(pRetryRequest.isNull());
}

class GetRetryRequestParam {
public:
    int statusCode;
    const char* pRequestUrl;
    const char* pLocationHeaderValue;
    bool authorizationHeaderExists;
    bool retryRequestExists;
    const char* pRetryRequestUrl;
    bool authorizationHeaderRemoved;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "statusCode : " + StringUtil::format("%d", statusCode) + "\n" +
                "pRequestUrl : " + (pRequestUrl == NULL ? "" : pRequestUrl) + "\n" +
                "pLocationHeaderValue : " + (pLocationHeaderValue == NULL ? "" : pLocationHeaderValue) + "\n" +
                "authorizationHeaderExists : " + StringUtil::boolToString(authorizationHeaderExists) + "\n" +
                "retryRequestExists : " + StringUtil::boolToString(retryRequestExists) + "\n" +
                "pRetryRequestUrl : " + (pRetryRequestUrl == NULL ? "" : pRetryRequestUrl) + "\n" +
                "authorizationHeaderRemoved : " + StringUtil::boolToString(authorizationHeaderRemoved) + "\n";
        return ret;
    }
};
static const GetRetryRequestParam GetRetryRequestData[] = {
    {   // statusCode == 200
        200, // statusCode
        NULL, // pRequetUrl
        NULL, // pLocationHeaderValue
        false, // authorizationHeaderExists
        false, // retryRequestExists
        NULL, // pRetryRequestUrl;
        false // authorizationHeaderRemoved
    },
    {   // statusCode == 400
        400,
        NULL,
        NULL,
        false,
        false,
        NULL,
        false
    },
    {   // statusCode == 500
        500,
        NULL,
        NULL,
        false,
        false,
        NULL,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header なし
        307,
        NULL,
        NULL,
        false,
        false,
        NULL,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header が空文字
        307,
        NULL,
        "",
        false,
        false,
        NULL,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header あり, scheme http->https
        307,
        RequestUrlHttp,
        LocationUrlHttp,
        false,
        true,
        LocationUrlHttp,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header あり, scheme https->http
        307,
        RequestUrlHttp,
        LocationUrlHttps,
        false,
        false,
        LocationUrlHttps,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header あり
        307,
        RequestUrlHttps,
        LocationUrlHttp,
        false,
        false,
        LocationUrlHttp,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header % encode 文字列
        307,
        RequestUrlHttp,
        LocationUrlEncode,
        false,
        true,
        LocationUrlHttp,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header あり, change scheme , Authorization Header あり
        307,
        RequestUrlHttps,
        LocationUrlHttp,
        true,
        false,
        NULL,
        false
    },
    {   // statusCode == 307, Method == GET, Location Header あり, change host, Authorization Header あり
        307,
        RequestUrlHttp,
        LocationUrlHttpHost,
        true,
        true,
        LocationUrlHttpHost,
        true
    },
    {   // statusCode == 307, Method == GET, Location Header あり, change port, Authorization Header あり

        307,
        RequestUrlHttpPort,
        LocationUrlHttpPort,
        true,
        true,
        LocationUrlHttpPort,
        true
    },
    {   // statusCode == 308, Method == GET, Location Header あり

        308,
        RequestUrlHttp,
        LocationUrlHttp,
        false,
        true,
        LocationUrlHttp,
        false
    },
    {   // statusCode == 301, Method == GET, Location Header あり

        301,
        RequestUrlHttp,
        LocationUrlHttp,
        false,
        true,
        LocationUrlHttp,
        false
    },
    {   // statusCode == 303, Method == GET, Location Header あり

        303,
        RequestUrlHttp,
        LocationUrlHttp,
        false,
        true,
        LocationUrlHttp,
        false
    }
};

class GetRetryRequestTest : public HttpEngineUnitTest,
public testing::WithParamInterface<GetRetryRequestParam> {
};
INSTANTIATE_TEST_CASE_P(HttpEngineUnitTest, GetRetryRequestTest,
        testing::ValuesIn(GetRetryRequestData));

TEST_P(GetRetryRequestTest, getRetryRequest_ReturnsRetryReponse_WhenCombinationOfStatusCodeAndHeader)
{
    GetRetryRequestParam& param = (GetRetryRequestParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given:
    Request::Builder requestBuilder;
    requestBuilder.httpGet();
    if (param.pRequestUrl != NULL) {
        requestBuilder.setUrl(param.pRequestUrl);
    }
    if (param.authorizationHeaderExists) {
        requestBuilder.setHeader(HeaderAuthorization, AuthorizationValue);
    }
    Request::Ptr pRequest = requestBuilder.build();
    HttpEngine httpEngine(NULL, pRequest, NULL);
    Response::Builder responseBuilder;
    if (param.pLocationHeaderValue != NULL) {
        responseBuilder.setHeader(HeaderLocation, param.pLocationHeaderValue);
    }
    Response::Ptr pResponse = responseBuilder.setCode(param.statusCode).setRequest(pRequest).build();

    // When: getRetryRequest
    Request::Ptr pRetryRequest = httpEngine.getRetryRequest(pResponse);

    // Then: return Request
    if (param.retryRequestExists) {
        EXPECT_FALSE(pRetryRequest.isNull());
        EXPECT_EQ(param.pRetryRequestUrl, pRetryRequest->getUrl());
        if (param.authorizationHeaderExists) {
            if (param.authorizationHeaderRemoved) {
                EXPECT_FALSE(pRetryRequest->hasHeader(HeaderAuthorization));
            } else {
                EXPECT_TRUE(pRetryRequest->hasHeader(HeaderAuthorization));
            }
        }
    } else {
        EXPECT_TRUE(pRetryRequest.isNull());
    }
}

TEST(HttpEngineUnitTest, getRetryRequest_ReturnsNull_WhenPostMethodAndStatusCodeIs307)
{
    // Given: POST Method and statusCode == 307
    Request::Builder requestBuilder;
    requestBuilder.httpPost();
    Request::Ptr pRequest = requestBuilder.build();
    HttpEngine httpEngine(NULL, pRequest, NULL);
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT).
            setRequest(pRequest).build();

    // When: getRetryRequest
    Request::Ptr pRetryRequest = httpEngine.getRetryRequest(pResponse);

    // Then: return NULL
    EXPECT_TRUE(pRetryRequest.isNull());
}

TEST(HttpEngineUnitTest, getRetryRequest_ReturnsNull_WhenPutMethodAndStatusCodeIs307)
{
    // Given: POST Method and statusCode == 307
    MediaType::Ptr pMediaType = new MediaType(DefaultContentType);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, DefaultRequestBody);
    Request::Builder requestBuilder;
    requestBuilder.httpPut(pRequestBody);
    Request::Ptr pRequest = requestBuilder.build();
    HttpEngine httpEngine(NULL, pRequest, NULL);
    Response::Builder responseBuilder;
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT).
            setRequest(pRequest).build();

    // When: getRetryRequest
    Request::Ptr pRetryRequest = httpEngine.getRetryRequest(pResponse);

    // Then: return NULL
    EXPECT_TRUE(pRetryRequest.isNull());
}

TEST(HttpEngineUnitTest, getRetryRequest_ReturnsRequest_WhenHeadMethodAndStatusCodeIs307AndExistLocation)
{
    // Given: HEAD MEthod and statusCode == 307
    Request::Builder requestBuilder;
    requestBuilder.httpHead();
    requestBuilder.setUrl(RequestUrlHttp);
    Request::Ptr pRequest = requestBuilder.build();
    HttpEngine httpEngine(NULL, pRequest, NULL);
    Response::Builder responseBuilder;
    responseBuilder.setHeader(HeaderLocation, LocationUrlHttp);
    Response::Ptr pResponse = responseBuilder.setCode(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT).
            setRequest(pRequest).build();

    // When: getRetryRequest
    Request::Ptr pRetryRequest = httpEngine.getRetryRequest(pResponse);

    // Then: return Request
    EXPECT_FALSE(pRetryRequest.isNull());
    EXPECT_EQ(LocationUrlHttp, pRetryRequest->getUrl());
}

TEST(HttpEngineUnitTest, getConnection_ReturnsNull_WhenBeforeExecute)
{
    // Given:
    HttpEngine httpEngine(NULL, NULL, NULL);

    // When: getConnection
    Connection::Ptr pConnection = httpEngine.getConnection();

    // Then: return NULL.
    ASSERT_TRUE(pConnection.isNull());
}

} /* namespace test */
} /* namespace easyhttpcpp */
