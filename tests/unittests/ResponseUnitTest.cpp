/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"

#include "ResponseBodyStreamInternal.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";
static const std::string HeaderName = "X-My-Header-Name";
static const std::string HeaderNameInLowerCase = "x-my-header-name";
static const std::string HeaderNameInUpperCase = "X-MY-HEADER-NAME";
static const std::string HeaderName1 = "X-My-Header-Name1";
static const std::string HeaderName1InLowerCase = "x-my-header-name1";
static const std::string HeaderName1InUpperCase = "X-MY-HEADER-NAME1";
static const std::string HeaderValueFoo = "foo";
static const std::string HeaderValueBar = "bar";
static const std::string HeaderValueBaz = "baz";
static const std::string HeaderDefaultValue = "default";
static const std::string Url = "http://www.example.com/path/index.html";
static const std::string Tag = "Test tag";
static const std::string Message = "test message";
static const int StatusCode199 = 199;
static const int StatusCodeOk = Poco::Net::HTTPResponse::HTTP_OK;
static const int StatusCode299 = 299;
static const int StatusCodeMultipleChoices = Poco::Net::HTTPResponse::HTTP_MULTIPLE_CHOICES;

TEST(ResponseUnitTest, copyConstructor_CopiesAllProperties)
{
    // Given: create Response instance
    // Body
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);
    // CacheControl
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    // Request
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.build();
    // Response
    Response::Builder responseBuilder;
    Response::Ptr pResponse1 = responseBuilder.build();
    Response::Ptr pResponse2 = responseBuilder.build();
    Response::Ptr pResponse3 = responseBuilder.build();
    responseBuilder.setCode(StatusCode299)
            .setBody(pBody)
            .setContentLength(Content.length())
            .setHasContentLength(true)
            .setHeader(HeaderName, HeaderValueFoo)
            .setMessage(Message)
            .setCacheControl(pCacheControl)
            .setRequest(pRequest)
            .setCacheResponse(pResponse1)
            .setNetworkResponse(pResponse2)
            .setPriorResponse(pResponse3)
            .setSentRequestSec(60)
            .setReceivedResponseSec(120);
    Response::Ptr pResponse = responseBuilder.build();

    // When: call Response() with response
    Response response(pResponse);

    // Then: all parameters are copied
    // Body
    EXPECT_EQ(pResponse->getBody(), response.getBody());
    // Code
    EXPECT_EQ(pResponse->getCode(), response.getCode());
    EXPECT_EQ(pResponse->getMessage(), response.getMessage());
    // ContentLength
    EXPECT_EQ(pResponse->hasContentLength(), response.hasContentLength());
    EXPECT_EQ(pResponse->getContentLength(), response.getContentLength());
    // CacheControl
    EXPECT_EQ(pResponse->getCacheControl(), response.getCacheControl());
    // Headers
    EXPECT_TRUE(response.hasHeader(HeaderName));
    EXPECT_TRUE(response.getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, response.getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(response.getHeaders()->getSize(), pResponse->getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, response.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    // Response
    EXPECT_EQ(pResponse->getCacheResponse(), response.getCacheResponse());
    EXPECT_EQ(pResponse->getNetworkResponse(), response.getNetworkResponse());
    EXPECT_EQ(pResponse->getPriorResponse(), response.getPriorResponse());
    // Sec
    EXPECT_EQ(pResponse->getSentRequestSec(), response.getSentRequestSec());
    EXPECT_EQ(pResponse->getReceivedResponseSec(), response.getReceivedResponseSec());
}

TEST(ResponseUnitTest, copyConstructor_ThrowsHttpIllegalArgumentException_WhenResponseIsNull)
{
    // Given: none   
    // When: call Response() with NULL
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(Response response(NULL), HttpIllegalArgumentException, 100700);
}

TEST(ResponseBuilderUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    // When: call Builder()
    Response::Builder builder;

    // Then: status code is 200, header and body are empty, other instance are NULL
    EXPECT_TRUE(builder.getBody().isNull());
    EXPECT_TRUE(builder.getCacheControl().isNull());
    EXPECT_EQ(StatusCodeOk, builder.getCode());
    EXPECT_EQ("", builder.getMessage());
    EXPECT_FALSE(builder.hasContentLength());
    EXPECT_EQ(-1, builder.getContentLength());
    EXPECT_TRUE(builder.getHeaders().isNull());
    EXPECT_TRUE(builder.getRequest().isNull());
    EXPECT_TRUE(builder.getCacheResponse().isNull());
    EXPECT_TRUE(builder.getNetworkResponse().isNull());
    EXPECT_TRUE(builder.getPriorResponse().isNull());
    EXPECT_EQ(0, builder.getSentRequestSec());
    EXPECT_EQ(0, builder.getReceivedResponseSec());
}

TEST(ResponseBuilderUnitTest, constructor_CopiesPropertiesFromResponse_WhenResponseIsPassed)
{
    // Given: create Response instance
    // Body
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);
    // CacheControl
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    // Request
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.build();
    // Response
    Response::Builder responseBuilder;
    Response::Ptr pResponse1 = responseBuilder.build();
    Response::Ptr pResponse2 = responseBuilder.build();
    Response::Ptr pResponse3 = responseBuilder.build();
    responseBuilder.setCode(StatusCode299)
            .setBody(pBody)
            .setContentLength(Content.length())
            .setHasContentLength(true)
            .setHeader(HeaderName, HeaderValueFoo)
            .setMessage(Message)
            .setCacheControl(pCacheControl)
            .setRequest(pRequest)
            .setCacheResponse(pResponse1)
            .setNetworkResponse(pResponse2)
            .setPriorResponse(pResponse3)
            .setSentRequestSec(60)
            .setReceivedResponseSec(120);
    Response::Ptr pResponse = responseBuilder.build();

    // When: call Builder() with response
    Response::Builder builder(pResponse);

    // Then: all parameters are copied 
    EXPECT_EQ(pBody, builder.getBody());
    EXPECT_EQ(StatusCode299, builder.getCode());
    EXPECT_EQ(Message, builder.getMessage());
    EXPECT_TRUE(builder.hasContentLength());
    EXPECT_EQ(Content.length(), builder.getContentLength());
    EXPECT_EQ(pCacheControl, builder.getCacheControl());
    EXPECT_TRUE(builder.getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(pResponse1, builder.getCacheResponse());
    EXPECT_EQ(pResponse2, builder.getNetworkResponse());
    EXPECT_EQ(pResponse3, builder.getPriorResponse());
    EXPECT_EQ(60, builder.getSentRequestSec());
    EXPECT_EQ(120, builder.getReceivedResponseSec());
}

TEST(ResponseBuilderUnitTest, constructor_ThrowsHttpIllegalArgumentException_WhenResponseIsNull)
{
    // Given: none
    // When: call Builder() with NULL
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(Response::Builder builder(NULL), HttpIllegalArgumentException, 100700);
}

TEST(ResponseBuilderUnitTest, build_ReturnsResponseInstance)
{
    // Given: none
    Response::Builder builder;

    // When: call build()
    Response::Ptr pResponse = builder.build();

    // Then: status code is 200, header and body are empty, other instance are NULL
    EXPECT_TRUE(pResponse->getBody().isNull());
    EXPECT_TRUE(pResponse->getCacheControl().isNull());
    EXPECT_EQ(StatusCodeOk, pResponse->getCode());
    EXPECT_EQ("", pResponse->getMessage());
    EXPECT_FALSE(pResponse->hasContentLength());
    EXPECT_EQ(-1, pResponse->getContentLength());
    EXPECT_FALSE(pResponse->getHeaders().isNull());
    EXPECT_EQ(0, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getRequest().isNull());
    EXPECT_TRUE(pResponse->getCacheResponse().isNull());
    EXPECT_TRUE(pResponse->getNetworkResponse().isNull());
    EXPECT_TRUE(pResponse->getPriorResponse().isNull());
    EXPECT_EQ(0, pResponse->getSentRequestSec());
    EXPECT_EQ(0, pResponse->getReceivedResponseSec());
    EXPECT_TRUE(pResponse->isSuccessful());
}

TEST(ResponseBuilderUnitTest, setBody_StoresBody)
{
    // Given: none
    Response::Builder builder;

    // When: call setBody()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);
    builder.setBody(pBody);

    // Then: body is stored, content length is set
    // Builder
    EXPECT_FALSE(builder.getBody().isNull());
    EXPECT_EQ(pBody, builder.getBody());
    EXPECT_TRUE(builder.hasContentLength());
    EXPECT_EQ(Content.length(), builder.getContentLength());
    EXPECT_TRUE(builder.getHeaders().isNull());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_FALSE(pResponse->getBody().isNull());
    EXPECT_EQ(pBody, pResponse->getBody());
    EXPECT_TRUE(pResponse->hasContentLength());
    EXPECT_EQ(Content.length(), pResponse->getContentLength());
    EXPECT_FALSE(pResponse->getHeaders().isNull());
    EXPECT_EQ(0, pResponse->getHeaders()->getSize());
}

TEST(ResponseBuilderUnitTest, setCode_StoresCode_WhenStatusCodeIs199)
{
    // Given: none
    Response::Builder builder;

    // When: call setCode()
    builder.setCode(StatusCode199);

    // Then: status code is stored
    // Builder
    EXPECT_EQ(StatusCode199, builder.getCode());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(StatusCode199, pResponse->getCode());
    EXPECT_EQ("", pResponse->getMessage());
    EXPECT_FALSE(pResponse->isSuccessful());
}

TEST(ResponseBuilderUnitTest, setCode_StoresCode_WhenStatusCodeIsOk)
{
    // Given: none
    Response::Builder builder;

    // When: call setCode()
    builder.setCode(StatusCode199).setCode(StatusCodeOk);

    // Then: status code is stored
    // Builder
    EXPECT_EQ(StatusCodeOk, builder.getCode());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(StatusCodeOk, pResponse->getCode());
    EXPECT_EQ("", pResponse->getMessage());
    EXPECT_TRUE(pResponse->isSuccessful());
}

TEST(ResponseBuilderUnitTest, setCode_StoresCode_WhenStatusCodeIs299)
{
    // Given: none
    Response::Builder builder;

    // When: call setCode()
    builder.setCode(StatusCode299);

    // Then: status code is stored
    // Builder
    EXPECT_EQ(StatusCode299, builder.getCode());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(StatusCode299, pResponse->getCode());
    EXPECT_EQ("", pResponse->getMessage());
    EXPECT_TRUE(pResponse->isSuccessful());
}

TEST(ResponseBuilderUnitTest, setCode_StoresCode_WhenStatusCodeIs300)
{
    // Given: none
    Response::Builder builder;

    // When: call setCode()
    builder.setCode(StatusCodeMultipleChoices);

    // Then: status code is stored
    // Builder
    EXPECT_EQ(StatusCodeMultipleChoices, builder.getCode());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(StatusCodeMultipleChoices, pResponse->getCode());
    EXPECT_EQ("", pResponse->getMessage());
    EXPECT_FALSE(pResponse->isSuccessful());
}

TEST(ResponseBuilderUnitTest, setMessage_StoresMessage)
{
    // Given: none
    Response::Builder builder;

    // When: call setMessage()
    builder.setMessage(Message);

    // Then: message is stored
    // Builder
    EXPECT_EQ(Message, builder.getMessage());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(Message, pResponse->getMessage());
}

TEST(ResponseBuilderUnitTest, setMessage_StoresMessage_WhenMessageIsEmpty)
{
    // Given: none
    Response::Builder builder;

    // When: call setMessage()
    builder.setMessage("");

    // Then: message is stored
    // Builder
    EXPECT_EQ("", builder.getMessage());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ("", pResponse->getMessage());
}

TEST(ResponseBuilderUnitTest, setHasContentLength_StoresValue)
{
    // Given: none
    Response::Builder builder;

    // When: call setHasContentLength()
    builder.setHasContentLength(true);

    // Then: value is stored
    // Builder
    EXPECT_TRUE(builder.hasContentLength());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->getBody().isNull());
    EXPECT_TRUE(pResponse->hasContentLength());
    EXPECT_EQ(-1, pResponse->getContentLength());
    EXPECT_FALSE(pResponse->getHeaders().isNull());
    EXPECT_EQ(0, pResponse->getHeaders()->getSize());
}

TEST(ResponseBuilderUnitTest, setContentLength_StoresValue)
{
    // Given: none
    Response::Builder builder;

    // When: call setContentLength()
    builder.setContentLength(100);

    // Then: value is stored
    // Builder
    EXPECT_EQ(100, builder.getContentLength());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->getBody().isNull());
    EXPECT_FALSE(pResponse->hasContentLength());
    EXPECT_EQ(100, pResponse->getContentLength());
    EXPECT_FALSE(pResponse->getHeaders().isNull());
    EXPECT_EQ(0, pResponse->getHeaders()->getSize());
}

TEST(ResponseBuilderUnitTest, setCacheControl_StoresCacheControl)
{
    // Given: none
    Response::Builder builder;

    // When: call setCacheControl()
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    builder.setCacheControl(pCacheControl);

    // Then: value is stored
    // Builder
    EXPECT_EQ(pCacheControl, builder.getCacheControl());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_EQ(pCacheControl, pResponse->getCacheControl());
}

TEST(ResponseBuilderUnitTest, setCacheControl_RemovesCacheControl_WhenCacheControlIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setCacheControl()
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    builder.setCacheControl(pCacheControl).setCacheControl(NULL);

    // Then: value is stored
    // Builder
    EXPECT_TRUE(builder.getCacheControl().isNull());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->getCacheControl().isNull());
}

TEST(ResponseBuilderUnitTest, setHeader_StoresHeader)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueFoo);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, setHeader_StoresHeader_WhenHeaderNameIsLowerCase)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueBar).setHeader(HeaderNameInLowerCase, HeaderValueBaz);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, setHeader_StoresHeader_WhenHeaderNameIsUpperCase)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueBar).setHeader(HeaderNameInUpperCase, HeaderValueBaz);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueBaz, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, setHeader_ThrowsHttpIllegalArgumentException_WhenNameIsEmpty)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeader() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setHeader("", HeaderValueFoo), HttpIllegalArgumentException, 100700);
}

TEST(ResponseBuilderUnitTest, setHeader_StoresHeader_WhenNameIsContentLength)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeader()
    builder.setHeader("Content-Length", "1024");

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ("1024", builder.getHeaders()->getValue("Content-Length", ""));
    // If set the "Content-Length" by setHeader(), not referenced by hasContentLength() and getContentLength()
    EXPECT_FALSE(builder.hasContentLength());
    EXPECT_EQ(-1, builder.getContentLength());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader("Content-Length"));
    EXPECT_EQ("1024", pResponse->getHeaderValue("Content-Length", ""));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has("Content-Length"));
    EXPECT_EQ("1024", pResponse->getHeaders()->getValue("Content-Length", ""));
    // If set the "Content-Length" by setHeader(), not referenced by hasContentLength() and getContentLength()
    EXPECT_FALSE(pResponse->hasContentLength());
    EXPECT_EQ(-1, pResponse->getContentLength());
}

TEST(ResponseBuilderUnitTest, addHeader_StoresHeader)
{
    // Given: none
    Response::Builder builder;

    // When: call addHeader()
    builder.addHeader(HeaderName, HeaderValueFoo);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, addHeader_StoresHeader_WhenAddMultipleValues)
{
    // Given: none
    Response::Builder builder;

    // When: call addHeader()
    builder.addHeader(HeaderName, HeaderValueBar).addHeader(HeaderName, HeaderValueBaz);

    // Then: value is stored
    // Builder
    EXPECT_EQ(2, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueBar, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBar, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBar, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBar, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBar, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBar, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(2, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueBar, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, addHeader_StoresHeader_WhenHeaderIsAlreadyAdded)
{
    // Given: none
    Response::Builder builder;
    builder.setHeader(HeaderName, HeaderValueFoo);

    // When: call addHeader()
    builder.addHeader(HeaderName, HeaderValueFoo);

    // Then: value is stored
    // Builder
    EXPECT_EQ(2, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(2, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, addHeader_StoresHeader_WhenHeaderNameIsLowerCase)
{
    // Given: none
    Response::Builder builder;

    // When: call addHeader()
    builder.addHeader(HeaderNameInLowerCase, HeaderValueFoo);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, addHeader_StoresHeader_WhenHeaderNameIsUpperCase)
{
    // Given: none
    Response::Builder builder;

    // When: call addHeader()
    builder.addHeader(HeaderNameInUpperCase, HeaderValueFoo);

    // Then: value is stored
    // Builder
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, addHeader_ThrowsHttpIllegalArgumentException_WhenNameIsEmpty)
{
    // Given: none
    Response::Builder builder;

    // When: call addHeader() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.addHeader("", HeaderValueFoo), HttpIllegalArgumentException, 100700);
}

TEST(ResponseBuilderUnitTest, setHeaders_StoresHeaders)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeaders()
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    builder.setHeaders(pHeaders);

    // Then: value is stored
    // Builder
    EXPECT_EQ(pHeaders, builder.getHeaders());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_TRUE(pResponse->hasHeader(HeaderName));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_NE(pHeaders, pResponse->getHeaders());
    EXPECT_EQ(1, pResponse->getHeaders()->getSize());
    EXPECT_TRUE(pResponse->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pResponse->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(ResponseBuilderUnitTest, setHeaders_RemovesHeaders_HeadersIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setHeaders() with NULL
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    builder.setHeaders(pHeaders).setHeaders(NULL);

    // Then: value is stored
    // Builder
    EXPECT_TRUE(builder.getHeaders().isNull());

    // Response
    Response::Ptr pResponse = builder.build();
    EXPECT_FALSE(pResponse->hasHeader(HeaderName));
    EXPECT_FALSE(pResponse->hasHeader(HeaderNameInLowerCase));
    EXPECT_FALSE(pResponse->hasHeader(HeaderNameInUpperCase));
    EXPECT_FALSE(pResponse->getHeaders().isNull());
    EXPECT_EQ(0, pResponse->getHeaders()->getSize());
}

TEST(ResponseBuilderUnitTest, setRequest_StoresInstance)
{
    // Given: none
    Response::Builder builder;

    // When: call setRequest()
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.build();
    builder.setRequest(pRequest);

    // Then: instance is stored
    // Builder
    EXPECT_EQ(pRequest, builder.getRequest());

    // Response
    EXPECT_EQ(pRequest, builder.build()->getRequest());
}

TEST(ResponseBuilderUnitTest, setRequest_RemovesInstance_WhenParameterIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setRequest()
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.build();
    builder.setRequest(pRequest).setRequest(NULL);

    // Then: instance is stored
    // Builder
    EXPECT_TRUE(builder.getRequest().isNull());

    // Response
    EXPECT_TRUE(builder.build()->getRequest().isNull());
}

TEST(ResponseBuilderUnitTest, setCacheResponse_StoresInstance)
{
    // Given: none
    Response::Builder builder;

    // When: call setCacheResponse()
    Response::Ptr pResponse = builder.build();
    builder.setCacheResponse(pResponse);

    // Then: instance is stored
    // Builder
    EXPECT_EQ(pResponse, builder.getCacheResponse());

    // Response
    EXPECT_EQ(pResponse, builder.build()->getCacheResponse());
}

TEST(ResponseBuilderUnitTest, setCacheResponse_RemovesInstance_WhenCacheResponsIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setCacheResponse()
    Response::Ptr pResponse = builder.build();
    builder.setCacheResponse(pResponse).setCacheResponse(NULL);

    // Then: instance is removed
    // Builder
    EXPECT_TRUE(builder.getCacheResponse().isNull());

    // Response
    EXPECT_TRUE(builder.build()->getCacheResponse().isNull());
}

TEST(ResponseBuilderUnitTest, setNetworkResponse_StoresInstance)
{
    // Given: none
    Response::Builder builder;

    // When: call setNetworkResponse()
    Response::Ptr pResponse = builder.build();
    builder.setNetworkResponse(pResponse);

    // Then: instance is stored
    // Builder
    EXPECT_EQ(pResponse, builder.getNetworkResponse());

    // Response
    EXPECT_EQ(pResponse, builder.build()->getNetworkResponse());
}

TEST(ResponseBuilderUnitTest, setNetworkResponse_RemovesInstance_WhenCacheResponsIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setNetworkResponse()
    Response::Ptr pResponse = builder.build();
    builder.setNetworkResponse(pResponse).setNetworkResponse(NULL);

    // Then: instance is removed
    // Builder
    EXPECT_TRUE(builder.getNetworkResponse().isNull());

    // Response
    EXPECT_TRUE(builder.build()->getNetworkResponse().isNull());
}

TEST(ResponseBuilderUnitTest, setPriorResponse_StoresInstance)
{
    // Given: none
    Response::Builder builder;

    // When: call setPriorResponse()
    Response::Ptr pResponse = builder.build();
    builder.setPriorResponse(pResponse);

    // Then: instance is stored
    // Builder
    EXPECT_EQ(pResponse, builder.getPriorResponse());

    // Response
    EXPECT_EQ(pResponse, builder.build()->getPriorResponse());
}

TEST(ResponseBuilderUnitTest, setPriorResponse_RemovesInstance_WhenCacheResponsIsNull)
{
    // Given: none
    Response::Builder builder;

    // When: call setPriorResponse()
    Response::Ptr pResponse = builder.build();
    builder.setPriorResponse(pResponse).setPriorResponse(NULL);

    // Then: instance is removed
    // Builder
    EXPECT_TRUE(builder.getPriorResponse().isNull());

    // Response
    EXPECT_TRUE(builder.build()->getPriorResponse().isNull());
}

TEST(ResponseBuilderUnitTest, setSentRequestSec_StoresValue)
{
    // Given: none
    Response::Builder builder;

    // When: call setSentRequestSec()
    builder.setSentRequestSec(600);

    // Then: value is stored
    // Builder
    EXPECT_EQ(600, builder.getSentRequestSec());

    // Response
    EXPECT_EQ(600, builder.build()->getSentRequestSec());
}

TEST(ResponseBuilderUnitTest, setReceivedResponseSec_StoresValue)
{
    // Given: none
    Response::Builder builder;

    // When: call setReceivedResponseSec()
    builder.setReceivedResponseSec(600);

    // Then: value is stored
    // Builder
    EXPECT_EQ(600, builder.getReceivedResponseSec());

    // Response
    EXPECT_EQ(600, builder.build()->getReceivedResponseSec());
}

} /* namespace test */
} /* namespace easyhttpcpp */
