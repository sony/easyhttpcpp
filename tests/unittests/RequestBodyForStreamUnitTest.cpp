/*
 * Copyright 2017 Sony Corporation
 */

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/RequestBody.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "RequestBodyForStream.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";
static const size_t VeryLargeContentSize = 1024 * 1024;

class MockStdOutputBuffer : public std::streambuf {
public:
    MOCK_METHOD1(overflow, int_type(int_type c));
};

class StubStdOutputStream : public std::ostream {
public:

    StubStdOutputStream(MockStdOutputBuffer& buffer) : std::ostream(&buffer)
    {
        exceptions(ios_base::failbit | std::ios_base::badbit);
    }
};

TEST(RequestBodyForStreamUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);

    // When: call RequestBodyForStream()
    RequestBodyForStream requestBody(pMediaType, is);

    // Then: parameters are set from content
    EXPECT_FALSE(requestBody.hasContentLength());
    EXPECT_EQ(-1, requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForStreamUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    std::istringstream is(Content);

    // When: call RequestBodyForStream()
    RequestBodyForStream requestBody(NULL, is);

    // Then: parameters are set from content
    EXPECT_FALSE(requestBody.hasContentLength());
    EXPECT_EQ(-1, requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForStreamUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    RequestBodyForStream requestBody(pMediaType, is);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

TEST(RequestBodyForStreamUnitTest, writeTo_WritesToOutputStream_WhenContentIsVeryLarge)
{
    // Given: none
    std::string content;
    while (content.length() < VeryLargeContentSize) {
        // 100 bytes
        content += "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789"
                "0123456789";
    }

    std::istringstream is(content);
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBodyForStream requestBody(pMediaType, is);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(content, os.str());
}

TEST(RequestBodyForStreamUnitTest, writeTo_ThrowsHttpExecutionException_WhenStdExceptionIsThrown)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    RequestBodyForStream requestBody(pMediaType, is);

    MockStdOutputBuffer mockBuffer;
    EXPECT_CALL(mockBuffer, overflow(testing::_)).Times(1).
            WillOnce(testing::Throw(std::logic_error("test error message")));

    // When: call writeTo()
    // Then: throws exception
    StubStdOutputStream os(mockBuffer);
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(requestBody.writeTo(os), HttpExecutionException, 100702);
}

TEST(RequestBodyForStreamUnitTest, writeTo_ThrowsHttpExecutionException_WhenPocoExceptionIsThrown)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    RequestBodyForStream requestBody(pMediaType, is);

    MockStdOutputBuffer mockBuffer;

    EXPECT_CALL(mockBuffer, overflow(testing::_)).Times(1).
            WillOnce(testing::Throw(Poco::Exception("test error message", 1234)));

    // When: call writeTo()
    // Then: throws exception
    StubStdOutputStream os(mockBuffer);
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(requestBody.writeTo(os), HttpExecutionException, 100702);
}

} /* namespace test */
} /* namespace easyhttpcpp */

