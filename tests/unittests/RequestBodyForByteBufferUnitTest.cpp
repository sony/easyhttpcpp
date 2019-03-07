/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "gtest/gtest.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/RequestBody.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"

#include "RequestBodyForByteBuffer.h"

#if defined(_WIN64)
#define SSIZE_MAX _I64_MAX
#elif defined(_WIN32)
#define SSIZE_MAX LONG_MAX
#endif

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";

TEST(RequestBodyForByteBufferUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    easyhttpcpp::common::ByteArrayBuffer buffer(Content);

    // When: call RequestBodyForByteBuffer()
    RequestBodyForByteBuffer requestBody(pMediaType, buffer);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForByteBufferUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    easyhttpcpp::common::ByteArrayBuffer buffer(Content);

    // When: call RequestBodyForByteBuffer()
    RequestBodyForByteBuffer requestBody(NULL, buffer);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForByteBufferUnitTest, constructor_ThrowsHttpIllegalArgumentException_WhenBufferDataSizeIsGreaterThanSsizeMax)
{
    // Given: set written data size to SSIZE_MAX + 1
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    easyhttpcpp::common::ByteArrayBuffer buffer(Content);
    size_t size = SSIZE_MAX;
    size++;
    buffer.setWrittenDataSize(size);

    // When: call RequestBodyForByteBuffer()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(RequestBodyForByteBuffer requestBody(pMediaType, buffer), HttpIllegalArgumentException, 100700);
}

TEST(RequestBodyForByteBufferUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    easyhttpcpp::common::ByteArrayBuffer buffer(Content);
    RequestBodyForByteBuffer requestBody(pMediaType, buffer);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

} /* namespace test */
} /* namespace easyhttpcpp */

