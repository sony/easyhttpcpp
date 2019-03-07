/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpUtil.h"

namespace easyhttpcpp {
namespace test {

static const std::string Url = "http://www.example.com/path/index.html";
#ifndef _WIN32
static const std::string GetMethodAndUrlHash = "d4ed7aaa0d5f04fad80bea5c3943741297b0a5d4c256fa1bb56dfeb00321e375";
static const std::string GetMethodHash = "14e30cd163c732912e048c4c837e15c4e90c062ebb795ab947d57706e2d10dd8";
#else
static const std::string GetMethodAndUrlHash = "efc0058bfc2909181f382018f9d1e1874333cac2";
static const std::string GetMethodHash = "f030bbbd32966cde41037b98a8849c46b76e4bc1";
#endif // !_WIN32

static const std::string HeaderName1 = "X-My-Header-Name1";
static const std::string HeaderName2 = "X-My-Header-Name2";
static const std::string HeaderName3 = "X-My-Header-Name3";
static const std::string HeaderValueFoo = "foo";
static const std::string HeaderValueBar = "bar";
static const std::string HeaderValue123 = "123";
static const std::string HeaderDefaultValue = "default";
static const std::string HeadersJson =
        "{\"X-My-Header-Name1\":\"foo\", \"X-My-Header-Name2\":\"bar\", \"X-My-Header-Name3\":123}";
static const std::string HeadersJsonArray = "[\"X-My-Header-Name1\", \"X-My-Header-Name2\", \"X-My-Header-Name3\"]";
static const std::string HeadersJsonNoSpace =
        "{\"X-My-Header-Name1\":\"foo\",\"X-My-Header-Name2\":\"bar\",\"X-My-Header-Name3\":\"123\"}";
static const std::string Key1 = "key1";
static const std::string CacheDataFileExtention = ".data";

class HttpMethodParam {
public:
    const Request::HttpMethod httpMethod;
    const char *httpMethodString;
};

static const HttpMethodParam HttpMethodToStringTestData[] = {
    {Request::HttpMethodDelete, "DELETE"},
    {Request::HttpMethodGet, "GET"},
    {Request::HttpMethodHead, "HEAD"},
    {Request::HttpMethodPatch, "PATCH"},
    {Request::HttpMethodPost, "POST"},
    {Request::HttpMethodPut, "PUT"}
};

class HttpMethodToStringTest : public testing::Test,
public testing::WithParamInterface<HttpMethodParam> {
};
INSTANTIATE_TEST_CASE_P(HttpUtilUnitTest, HttpMethodToStringTest,
        testing::ValuesIn(HttpMethodToStringTestData));

TEST_P(HttpMethodToStringTest, httpMethodToString_ReturnsExpectedString)
{
    HttpMethodParam& param = (HttpMethodParam&) GetParam();
    EXPECT_EQ(param.httpMethodString, HttpUtil::httpMethodToString(param.httpMethod));
}

TEST(HttpUtilUnitTest, tryParseDate_ReturnsTrueAndSetTimestap_WhenDateFormatIsRFC822UpdatedByRFC1123)
{
    // Given: none
    // When: call tryParseDate()
    // Then: returns true and timestamp
    Poco::Timestamp timestamp(Poco::Timestamp::TIMEVAL_MIN);
    std::string date = "Sun, 06 Nov 1994 08:49:37 GMT";
    EXPECT_TRUE(HttpUtil::tryParseDate(date, timestamp));
    EXPECT_EQ(784111777, timestamp.epochTime());
}

TEST(HttpUtilUnitTest, tryParseDate_ReturnsTrueAndSetTimestap_WhenDateFormatIsAnsiC)
{
    // Given: none
    // When: call tryParseDate()
    // Then: returns true and timestamp
    Poco::Timestamp timestamp(Poco::Timestamp::TIMEVAL_MIN);
    std::string date = "Sun Nov  6 08:49:37 1994";
    EXPECT_TRUE(HttpUtil::tryParseDate(date, timestamp));
    EXPECT_EQ(784111777, timestamp.epochTime());
}

TEST(HttpUtilUnitTest, tryParseDate_ReturnsFalse_WhenDateFormatIsUnsupported)
{
    // Given: none
    // When: call tryParseDate()
    // Then: returns true and timestamp
    Poco::Timestamp timestamp(Poco::Timestamp::TIMEVAL_MIN);
    // RFC3339 format
    std::string date = "1994-11-06T08:49:37.00Z";
    EXPECT_FALSE(HttpUtil::tryParseDate(date, timestamp));
}

TEST(HttpUtilUnitTest, makeCacheKeyWithRequest_ReturnsHashedValue)
{
    // Given: HTTP method and URL are set
    Request::Builder builder;
    Request::Ptr request = builder.httpGet().setUrl(Url).build();

    // When: call makeCacheKey()
    std::string key = HttpUtil::makeCacheKey(request);
    // Then: returns hashed value of HTTP method + URL
    EXPECT_EQ(GetMethodAndUrlHash, key);
}

TEST(HttpUtilUnitTest, makeCacheKeyWithRequest_ReturnsHashedValue_WhenUrlIsNotSet)
{
    // Given: HTTP method is set, URL is not set
    Request::Builder builder;
    Request::Ptr request = builder.httpGet().build();

    // When: call makeCacheKey()
    std::string key = HttpUtil::makeCacheKey(request);
    // Then: returns hashed value of HTTP method
    EXPECT_EQ(GetMethodHash, key);
}

TEST(HttpUtilUnitTest, makeCacheKeyWithHttpMethodAndUrl_ReturnsHashedValue)
{
    // Given: none
    // When: call makeCacheKey()
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, Url);
    // Then: returns hashed value of HTTP method + URL
    EXPECT_EQ(GetMethodAndUrlHash, key);
}

TEST(HttpUtilUnitTest, makeCacheKeyWithHttpMethodAndUrl_ReturnsHashedValue_WhenUrlIsEmpty)
{
    // Given: none
    // When: call makeCacheKey()
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, "");
    // Then: returns hashed value of HTTP method
    EXPECT_EQ(GetMethodHash, key);
}

TEST(HttpUtilUnitTest, exchangeJsonStrToHeaders_ReturnsHeadersInstance)
{
    // Given: none
    // When: call exchangeJsonStrToHeaders()
    Headers::Ptr headers = HttpUtil::exchangeJsonStrToHeaders(HeadersJson);

    // Then: returns Headers instance from JSON
    EXPECT_EQ(3, headers->getSize());
    EXPECT_TRUE(headers->has(HeaderName1));
    EXPECT_EQ(HeaderValueFoo, headers->getValue(HeaderName1, ""));
    EXPECT_TRUE(headers->has(HeaderName2));
    EXPECT_EQ(HeaderValueBar, headers->getValue(HeaderName2, ""));
    EXPECT_TRUE(headers->has(HeaderName3));
    EXPECT_EQ(HeaderValue123, headers->getValue(HeaderName3, ""));
}

TEST(HttpUtilUnitTest, exchangeJsonStrToHeaders_ReturnsHeadersInstance_WhenJsonIsEmpty)
{
    // Given: none
    // When: call exchangeJsonStrToHeaders()
    Headers::Ptr headers = HttpUtil::exchangeJsonStrToHeaders("{}");

    // Then: returns Headers instance from JSON
    EXPECT_EQ(0, headers->getSize());
}

TEST(HttpUtilUnitTest, exchangeJsonStrToHeaders_ReturnsNull_WhenJsonIsNotObject)
{
    // Given: none
    // When: call exchangeJsonStrToHeaders()
    Headers::Ptr headers = HttpUtil::exchangeJsonStrToHeaders(HeadersJsonArray);
    // Then: returns NULL
    EXPECT_TRUE(headers.isNull());
}

TEST(HttpUtilUnitTest, exchangeJsonStrToHeaders_ReturnsNull_WhenFormatIsInvalid)
{
    // Given: none
    // When: call exchangeJsonStrToHeaders()
    Headers::Ptr headers = HttpUtil::exchangeJsonStrToHeaders("invalid json string");
    // Then: returns NULL
    EXPECT_TRUE(headers.isNull());
}

TEST(HttpUtilUnitTest, exchangeHeadersToJsonStr_ReturnsJsonString)
{
    // Given: Headers has some values
    Headers headers;
    headers.add(HeaderName1, HeaderValueFoo);
    headers.add(HeaderName2, HeaderValueBar);
    headers.add(HeaderName3, HeaderValue123);
    Headers::Ptr pHeaders = new Headers(headers);

    // When: call exchangeHeadersToJsonStr()
    std::string json = HttpUtil::exchangeHeadersToJsonStr(pHeaders);

    // Then: returns JSON string
    EXPECT_EQ(HeadersJsonNoSpace, json);
}

TEST(HttpUtilUnitTest, exchangeHeadersToJsonStr_ReturnsJsonStringAsEmpty_WhenHeadersIsEmpty)
{
    // Given: Headers is empty
    Headers headers;
    Headers::Ptr pHeaders = new Headers(headers);

    // When: call exchangeHeadersToJsonStr()
    std::string json = HttpUtil::exchangeHeadersToJsonStr(pHeaders);

    // Then: returns JSON string
    EXPECT_EQ("{}", json);
}

TEST(HttpUtilUnitTest, exchangeHeadersToJsonStr_ReturnsEmptyString_WhenHeadersIsNull)
{
    // Given: none
    // When: call exchangeHeadersToJsonStr()
    std::string json = HttpUtil::exchangeHeadersToJsonStr(NULL);

    // Then: returns JSON string
    EXPECT_EQ("", json);
}

TEST(HttpUtilUnitTest, makeCachedResponseBodyFilename_ReturnsFilename_WhenSpecifiedCacheRootDirAndKey)
{
    std::string cacheRootDirStr = std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + "/test/";
    Poco::Path cacheRootDir(cacheRootDirStr);
    std::string expectedFileName = Poco::Path(cacheRootDirStr + Key1 + CacheDataFileExtention).toString();
    EXPECT_EQ(expectedFileName,
            HttpUtil::makeCachedResponseBodyFilename(cacheRootDir, Key1));
}

} /* namespace test */
} /* namespace easyhttpcpp */

