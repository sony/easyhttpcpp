/*
 * Copyright 2018 Sony Corporation
 */

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/RequestBody.h"
#include "EasyHttpCppAssertions.h"

#include "RequestBodyForSharedPtrStream.h"

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

TEST(RequestBodyForSharedPtrStreamUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);

    // When: call RequestBodyForSharedPtrStream()
    RequestBodyForSharedPtrStream requestBody(pMediaType, pContent);

    // Then: parameters are set from content
    EXPECT_FALSE(requestBody.hasContentLength());
    EXPECT_EQ(-1, requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForSharedPtrStreamUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);

    // When: call RequestBodyForSharedPtrStream()
    RequestBodyForSharedPtrStream requestBody(NULL, pContent);

    // Then: parameters are set from content
    EXPECT_FALSE(requestBody.hasContentLength());
    EXPECT_EQ(-1, requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForSharedPtrStreamUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);
    RequestBodyForSharedPtrStream requestBody(pMediaType, pContent);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

TEST(RequestBodyForSharedPtrStreamUnitTest, writeTo_WritesToOutputStream_WhenContentIsVeryLarge)
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

    Poco::SharedPtr<std::istream> pContent = new std::istringstream(content);
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBodyForSharedPtrStream requestBody(pMediaType, pContent);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(content, os.str());
}

TEST(RequestBodyForSharedPtrStreamUnitTest, writeTo_ThrowsHttpExecutionException_WhenStdExceptionIsThrown)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);
    RequestBodyForSharedPtrStream requestBody(pMediaType, pContent);

    MockStdOutputBuffer mockBuffer;
    EXPECT_CALL(mockBuffer, overflow(testing::_)).Times(1).
            WillOnce(testing::Throw(std::logic_error("test error message")));

    // When: call writeTo()
    // Then: throws exception
    StubStdOutputStream os(mockBuffer);
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(requestBody.writeTo(os), HttpExecutionException, 100702);
}

TEST(RequestBodyForSharedPtrStreamUnitTest, writeTo_ThrowsHttpExecutionException_WhenPocoExceptionIsThrown)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);
    RequestBodyForSharedPtrStream requestBody(pMediaType, pContent);

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

