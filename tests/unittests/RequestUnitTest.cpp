/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";
static const std::string HeaderName = "X-My-Header-Name";
static const std::string HeaderNameInLowerCase = "x-my-header-name";
static const std::string HeaderNameInUpperCase = "X-MY-HEADER-NAME";
static const std::string HeaderValueFoo = "foo";
static const std::string HeaderValueBar = "bar";
static const std::string HeaderValueBaz = "baz";
static const std::string HeaderDefaultValue = "default";
static const std::string Url = "http://www.example.com/path/index.html";
static const std::string Tag = "Test tag";

TEST(RequestUnitTest, constructor_CopiesPropertiesFromBuilder_WhenBuilderIsPassed)
{
    // Given: set parameters to builder
    // Body
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    // Headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    // CacheControl
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    // Tag
    const char* pTag = "Test tag";

    Request::Builder builder;
    builder
            .httpPost(pBody)
            .setHeaders(pHeaders)
            .setUrl(Url)
            .setTag(pTag)
            .setCacheControl(pCacheControl);

    // When: call Request() with builder
    Request request(builder);

    // Then: all parameters are copied
    // Tag
    EXPECT_EQ(builder.getTag(), request.getTag());
    // CacheControl
    EXPECT_EQ(builder.getCacheControl(), request.getCacheControl());
    // Method
    EXPECT_EQ(builder.getMethod(), request.getMethod());
    // URL
    EXPECT_EQ(builder.getUrl(), request.getUrl());
    // Headers
    EXPECT_TRUE(request.hasHeader(HeaderName));
    EXPECT_EQ(HeaderValueFoo, request.getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_TRUE(request.getHeaders()->has(HeaderName));
    EXPECT_EQ(1, request.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, request.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    // Body
    EXPECT_FALSE(request.getBody().isNull());
    std::ostringstream stream1;
    request.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());
}

TEST(RequestUnitTest, copyConstructor_CopiesAllProperties)
{
    // Given: build request
    // Body
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    // Headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    // CacheControl
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();

    Request::Builder builder1;
    Request::Ptr pRequest1 = builder1
            .httpPost(pBody)
            .setHeaders(pHeaders)
            .setUrl(Url)
            .setTag(Tag.c_str())
            .setCacheControl(pCacheControl)
            .build();

    // When: call Request() with request
    Request request(pRequest1);

    // Then: all parameters are copied
    // Tag
    EXPECT_EQ(pRequest1->getTag(), request.getTag());
    // CacheControl
    EXPECT_EQ(pRequest1->getCacheControl(), request.getCacheControl());
    // Method
    EXPECT_EQ(pRequest1->getMethod(), request.getMethod());
    // URL
    EXPECT_EQ(pRequest1->getUrl(), request.getUrl());
    // Headers
    EXPECT_TRUE(request.hasHeader(HeaderName));
    EXPECT_EQ(HeaderValueFoo, request.getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_TRUE(request.getHeaders()->has(HeaderName));
    EXPECT_EQ(1, request.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, request.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    // Body
    EXPECT_FALSE(request.getBody().isNull());
    std::ostringstream stream1;
    request.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());
}

TEST(RequestUnitTest, copyConstructor_ThrowsHttpIllegalArgumentException_WhenRequestIsNull)
{
    // Given: none   
    // When: call Request() with NULL
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(Request request(NULL), HttpIllegalArgumentException, 100700);
}

TEST(RequestBuilderUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    // When: call Builder()
    Request::Builder builder;

    // Then: HTTP method is GET, header and body are empty
    EXPECT_EQ(Request::HttpMethodGet, builder.getMethod());
    EXPECT_TRUE(builder.getBody().isNull());
    EXPECT_TRUE(builder.getCacheControl().isNull());
    EXPECT_TRUE(builder.getHeaders().isNull());
    EXPECT_EQ(NULL, builder.getTag());
    EXPECT_EQ("", builder.getUrl());
}

TEST(RequestBuilderUnitTest, constructor_CopiesPropertiesFromRequest_WhenRequestIsPassed)
{
    // Given: build request
    // Body
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    // Headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    // CacheControl
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();

    Request::Builder builder1;
    Request::Ptr pRequest1 = builder1
            .httpPost(pBody)
            .setHeaders(pHeaders)
            .setUrl(Url)
            .setTag(Tag.c_str())
            .setCacheControl(pCacheControl)
            .build();

    // When: call Builder() with request
    Request::Builder builder2(pRequest1);

    // Then: all parameters are copied
    // Tag
    EXPECT_EQ(builder1.getTag(), builder2.getTag());
    // CacheControl
    EXPECT_EQ(builder1.getCacheControl(), builder2.getCacheControl());
    // Method
    EXPECT_EQ(builder1.getMethod(), builder2.getMethod());
    // URL
    EXPECT_EQ(builder1.getUrl(), builder2.getUrl());
    // Headers
    EXPECT_TRUE(builder2.getHeaders()->has(HeaderName));
    EXPECT_EQ(1, builder2.getHeaders()->getSize());
    EXPECT_EQ(HeaderValueFoo, builder2.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    // Body
    EXPECT_FALSE(builder2.getBody().isNull());
    std::ostringstream stream1;
    builder2.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());
}

TEST(RequestBuilderUnitTest, constructor_ThrowsHttpIllegalArgumentException_WhenRequestIsNull)
{
    // Given: none

    // When: call Builder() with NULL
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(new Request::Builder(NULL), HttpIllegalArgumentException, 100700);
}

TEST(RequestBuilderUnitTest, build_ReturnsRequestInstance)
{
    // Given: none
    Request::Builder builder;

    // When: call build()
    Request::Ptr pRequest = builder.build();

    // Then: HTTP method is GET, header and body are empty
    EXPECT_EQ(Request::HttpMethodGet, pRequest->getMethod());
    EXPECT_TRUE(pRequest->getBody().isNull());
    EXPECT_FALSE(pRequest->getCacheControl().isNull());
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
}

TEST(RequestBuilderUnitTest, httpDeleteWithBody_SetsMethodAndStoresBody)
{
    // Given: none
    Request::Builder builder;

    // When: call httpDelete()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    builder.httpDelete(pBody);

    // Then: HTTP method is DELETE, header is empty, body is set
    // Method
    EXPECT_EQ(Request::HttpMethodDelete, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_FALSE(builder.getBody().isNull());
    std::ostringstream stream1;
    builder.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodDelete, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_FALSE(pRequest->getBody().isNull());

    EXPECT_EQ(ContentType, pRequest->getBody()->getMediaType()->toString());
    EXPECT_TRUE(pRequest->getBody()->hasContentLength());
    EXPECT_EQ(Content.length(), pRequest->getBody()->getContentLength());
    std::ostringstream stream2;
    pRequest->getBody()->writeTo(stream2);
    EXPECT_EQ(Content, stream2.str());
}

TEST(RequestBuilderUnitTest, httpDelete_SetsMethod)
{
    // Given: none
    Request::Builder builder;

    // When: call httpDelete()
    builder.httpDelete();

    // Then: HTTP method is DELETE, header and body are empty
    // Method
    EXPECT_EQ(Request::HttpMethodDelete, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_TRUE(builder.getBody().isNull());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodDelete, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_TRUE(pRequest->getBody().isNull());
}

TEST(RequestBuilderUnitTest, httpGet_SetsMethod)
{
    // Given: none
    Request::Builder builder;

    // When: call httpGet()
    builder.httpGet();

    // Then: HTTP method is GET, header and body are empty
    // Method
    EXPECT_EQ(Request::HttpMethodGet, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_TRUE(builder.getBody().isNull());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodGet, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_TRUE(pRequest->getBody().isNull());
}

TEST(RequestBuilderUnitTest, httpHead_SetsMethod)
{
    // Given: none
    Request::Builder builder;

    // When: call httpHead()
    builder.httpHead();

    // Then: HTTP method is HEAD, header and body are empty
    // Method
    EXPECT_EQ(Request::HttpMethodHead, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_TRUE(builder.getBody().isNull());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodHead, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_TRUE(pRequest->getBody().isNull());
}

TEST(RequestBuilderUnitTest, httpPatchWithBody_SetsMethodAndStoresBody)
{
    // Given: none
    Request::Builder builder;

    // When: call httpPatch()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    builder.httpPatch(pBody);

    // Then: HTTP method is PATCH, header is empty, body is set
    // Method
    EXPECT_EQ(Request::HttpMethodPatch, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_FALSE(builder.getBody().isNull());
    std::ostringstream stream1;
    builder.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodPatch, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_FALSE(pRequest->getBody().isNull());

    EXPECT_EQ(ContentType, pRequest->getBody()->getMediaType()->toString());
    EXPECT_TRUE(pRequest->getBody()->hasContentLength());
    EXPECT_EQ(Content.length(), pRequest->getBody()->getContentLength());
    std::ostringstream stream2;
    pRequest->getBody()->writeTo(stream2);
    EXPECT_EQ(Content, stream2.str());
}

TEST(RequestBuilderUnitTest, httpPostWithBody_SetsMethodAndStoresBody)
{
    // Given: none
    Request::Builder builder;

    // When: call httpPost()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    builder.httpPost(pBody);

    // Then: HTTP method is POST, header is empty, body is set
    // Method
    EXPECT_EQ(Request::HttpMethodPost, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_FALSE(builder.getBody().isNull());
    std::ostringstream stream1;
    builder.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodPost, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_FALSE(pRequest->getBody().isNull());

    EXPECT_EQ(ContentType, pRequest->getBody()->getMediaType()->toString());
    EXPECT_TRUE(pRequest->getBody()->hasContentLength());
    EXPECT_EQ(Content.length(), pRequest->getBody()->getContentLength());
    std::ostringstream stream2;
    pRequest->getBody()->writeTo(stream2);
    EXPECT_EQ(Content, stream2.str());
}

TEST(RequestBuilderUnitTest, httpPost_SetsMethod)
{
    // Given: none
    Request::Builder builder;

    // When: call httpPost()
    builder.httpPost();

    // Then: HTTP method is POST, header and body are empty
    // Method
    EXPECT_EQ(Request::HttpMethodPost, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_TRUE(builder.getBody().isNull());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodPost, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_TRUE(pRequest->getBody().isNull());
}

TEST(RequestBuilderUnitTest, httpPost_SetsMethodAndRemovedBody_WhenBodyWasSet)
{
    // Given: body is set
    Request::Builder builder;
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    builder.httpPost(pBody);

    // When: call httpPost()
    builder.httpPost();

    // Then: HTTP method is POST, header and body are empty
    // Method
    EXPECT_EQ(Request::HttpMethodPost, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_FALSE(builder.getBody().isNull());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodPost, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_FALSE(pRequest->getBody().isNull());
}

TEST(RequestBuilderUnitTest, httpPutWithBody_SetsMethod)
{
    // Given: none
    Request::Builder builder;

    // When: call httpPut()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBody::Ptr pBody = RequestBody::create(pMediaType, Content);
    builder.httpPut(pBody);

    // Then: HTTP method is PUT, header is empty, body is set
    // Method
    EXPECT_EQ(Request::HttpMethodPut, builder.getMethod());
    // Headers
    EXPECT_TRUE(builder.getHeaders().isNull());
    // Body
    EXPECT_FALSE(builder.getBody().isNull());
    std::ostringstream stream1;
    builder.getBody()->writeTo(stream1);
    EXPECT_EQ(Content, stream1.str());

    Request::Ptr pRequest = builder.build();
    // Method
    EXPECT_EQ(Request::HttpMethodPut, pRequest->getMethod());
    // Headers
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
    // Body
    EXPECT_FALSE(pRequest->getBody().isNull());

    EXPECT_EQ(ContentType, pRequest->getBody()->getMediaType()->toString());
    EXPECT_TRUE(pRequest->getBody()->hasContentLength());
    EXPECT_EQ(Content.length(), pRequest->getBody()->getContentLength());
    std::ostringstream stream2;
    pRequest->getBody()->writeTo(stream2);
    EXPECT_EQ(Content, stream2.str());
}

TEST(RequestBuilderUnitTest, setCacheControl_StoresCacheControl)
{
    // Given: none
    Request::Builder builder;

    // When: call setCacheControl()
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    builder.setCacheControl(pCacheControl);

    // Then: CacheControl is set
    EXPECT_EQ(pCacheControl, builder.getCacheControl());

    Request::Ptr pRequest = builder.build();
    EXPECT_EQ(pCacheControl, pRequest->getCacheControl());
}

TEST(RequestBuilderUnitTest, setCacheControl_RemovesCacheControl_WhenCacheControlIsNull)
{
    // Given: none
    Request::Builder builder;

    // When: call setCacheControl() with NULL
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();
    builder.setCacheControl(pCacheControl).setCacheControl(NULL);

    // Then: CacheControl is NULL
    EXPECT_TRUE(builder.getCacheControl().isNull());

    Request::Ptr pRequest = builder.build();
    EXPECT_FALSE(pRequest->getCacheControl().isNull());
}

TEST(RequestBuilderUnitTest, setHeader_StoresHeader)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueFoo);

    // Then: Header is set
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_TRUE(builder.getHeaders()->has(HeaderName));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInLowerCase));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    Request::Ptr pRequest = builder.build();
    EXPECT_TRUE(pRequest->hasHeader(HeaderName));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pRequest->getHeaders()->getSize());
    EXPECT_TRUE(pRequest->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(RequestBuilderUnitTest, setHeader_StoresHeader_WhenHeaderNameIsLowerCase)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueBar).setHeader(HeaderNameInLowerCase, HeaderValueBaz);

    // Then: Header is set
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_TRUE(builder.getHeaders()->has(HeaderName));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInLowerCase));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    Request::Ptr pRequest = builder.build();
    EXPECT_TRUE(pRequest->hasHeader(HeaderName));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pRequest->getHeaders()->getSize());
    EXPECT_TRUE(pRequest->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(RequestBuilderUnitTest, setHeader_StoresHeader_WhenHeaderNameIsUpperCase)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeader()
    builder.setHeader(HeaderName, HeaderValueBar).setHeader(HeaderNameInUpperCase, HeaderValueBaz);

    // Then: Header is set
    EXPECT_EQ(1, builder.getHeaders()->getSize());
    EXPECT_TRUE(builder.getHeaders()->has(HeaderName));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInLowerCase));
    EXPECT_TRUE(builder.getHeaders()->has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, builder.getHeaders()->getValue(HeaderNameInUpperCase, HeaderDefaultValue));

    Request::Ptr pRequest = builder.build();
    EXPECT_TRUE(pRequest->hasHeader(HeaderName));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_EQ(1, pRequest->getHeaders()->getSize());
    EXPECT_TRUE(pRequest->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueBaz, pRequest->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(RequestBuilderUnitTest, setHeader_ThrowsHttpIllegalArgumentException_WhenNameIsEmpty)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeader() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setHeader("", HeaderValueFoo), HttpIllegalArgumentException, 100700);
}

TEST(RequestBuilderUnitTest, setHeaders_StoresHeaders)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeaders()
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    builder.setHeaders(pHeaders);

    // Then: Headers is set
    EXPECT_EQ(pHeaders, builder.getHeaders());

    Request::Ptr pRequest = builder.build();
    EXPECT_TRUE(pRequest->hasHeader(HeaderName));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInLowerCase));
    EXPECT_TRUE(pRequest->hasHeader(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderNameInLowerCase, HeaderDefaultValue));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaderValue(HeaderNameInUpperCase, HeaderDefaultValue));
    EXPECT_NE(pHeaders, pRequest->getHeaders());
    EXPECT_EQ(1, pRequest->getHeaders()->getSize());
    EXPECT_TRUE(pRequest->getHeaders()->has(HeaderName));
    EXPECT_EQ(HeaderValueFoo, pRequest->getHeaders()->getValue(HeaderName, HeaderDefaultValue));
}

TEST(RequestBuilderUnitTest, setHeaders_RemovesHeaders_HeadersIsNull)
{
    // Given: none
    Request::Builder builder;

    // When: call setHeaders() with NULL
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add(HeaderName, HeaderValueFoo);
    builder.setHeaders(pHeaders).setHeaders(NULL);

    // Then: Headers is empty
    EXPECT_TRUE(builder.getHeaders().isNull());

    Request::Ptr pRequest = builder.build();
    EXPECT_FALSE(pRequest->hasHeader(HeaderName));
    EXPECT_FALSE(pRequest->hasHeader(HeaderNameInLowerCase));
    EXPECT_FALSE(pRequest->hasHeader(HeaderNameInUpperCase));
    EXPECT_FALSE(pRequest->getHeaders().isNull());
    EXPECT_EQ(0, pRequest->getHeaders()->getSize());
}

TEST(RequestBuilderUnitTest, setTag_StoresTag)
{
    // Given: none
    Request::Builder builder;

    // When: call setTag()
    builder.setTag(Tag.c_str());

    // Then: Tag is set
    EXPECT_EQ(Tag.c_str(), builder.getTag());

    Request::Ptr pRequest = builder.build();
    EXPECT_EQ(Tag.c_str(), pRequest->getTag());
}

TEST(RequestBuilderUnitTest, setTag_RemovesTag_WhenTagIsNull)
{
    // Given: none
    Request::Builder builder;

    // When: call setTag() with NULL
    builder.setTag(Tag.c_str()).setTag(NULL);

    // Then: Tag is NULL
    EXPECT_EQ(NULL, builder.getTag());

    Request::Ptr pRequest = builder.build();
    EXPECT_EQ(NULL, pRequest->getTag());
}

TEST(RequestBuilderUnitTest, setUrl_StoresUrl)
{
    // Given: none
    Request::Builder builder;

    // When: call setUrl()
    builder.setUrl(Url);

    // Then: URL is set
    EXPECT_EQ(Url, builder.getUrl());

    Request::Ptr pRequest = builder.build();
    EXPECT_EQ(Url, pRequest->getUrl());
}

TEST(RequestBuilderUnitTest, setUrl_ThrowsHttpIllegalArgumentException_WhenUrlIsEmpty)
{
    // Given: none
    Request::Builder builder;

    // When: call setUrl() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setUrl(""), HttpIllegalArgumentException, 100700);
}

TEST(RequestBuilderUnitTest, setUrl_ThrowsHttpIllegalArgumentException_WhenPathContainsDecodeImpossibleString)
{
    // Given: none
    Request::Builder builder;

    // When: call setUrl() with can not decode string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(builder.setUrl("http://localhost:9982/%?a=0"), HttpIllegalArgumentException, 100700);
}

TEST(RequestBuilderUnitTest, setUrl_ThrowsHttpIllegalArgumentException_WhenFragmentContainsDecodeImpossibleString)
{
    // Given: none
    Request::Builder builder;

    // When: call setUrl() with can not decode string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(builder.setUrl("http://localhost:9982/path?a=0#%"), HttpIllegalArgumentException,
            100700);
}

} /* namespace test */
} /* namespace easyhttpcpp */
