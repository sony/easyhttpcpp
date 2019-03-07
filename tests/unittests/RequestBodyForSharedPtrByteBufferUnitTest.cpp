/*
 * Copyright 2018 Sony Corporation
 */

#include <limits.h>

#include "gtest/gtest.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/RequestBody.h"
#include "EasyHttpCppAssertions.h"

#include "RequestBodyForSharedPtrByteBuffer.h"

#if defined(_WIN64)
#define SSIZE_MAX _I64_MAX
#elif defined(_WIN32)
#define SSIZE_MAX LONG_MAX
#endif

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";

TEST(RequestBodyForSharedPtrByteBufferUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<ByteArrayBuffer> pContent = new ByteArrayBuffer(Content);

    // When: call RequestBodyForSharedPtrByteBuffer()
    RequestBodyForSharedPtrByteBuffer requestBody(pMediaType, pContent);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForSharedPtrByteBufferUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    Poco::SharedPtr<ByteArrayBuffer> pContent = new ByteArrayBuffer(Content);

    // When: call RequestBodyForSharedPtrByteBuffer()
    RequestBodyForSharedPtrByteBuffer requestBody(NULL, pContent);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForSharedPtrByteBufferUnitTest,
        constructor_ThrowsHttpIllegalArgumentException_WhenBufferDataSizeIsGreaterThanSsizeMax)
{
    // Given: set written data size to SSIZE_MAX + 1
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<ByteArrayBuffer> pContent = new ByteArrayBuffer(Content);
    size_t size = SSIZE_MAX;
    size++;
    pContent->setWrittenDataSize(size);

    // When: call RequestBodyForSharedPtrByteBuffer()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(RequestBodyForSharedPtrByteBuffer requestBody(pMediaType, pContent), HttpIllegalArgumentException,
            100700);
}

TEST(RequestBodyForSharedPtrByteBufferUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<ByteArrayBuffer> pContent = new ByteArrayBuffer(Content);
    RequestBodyForSharedPtrByteBuffer requestBody(pMediaType, pContent);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

} /* namespace test */
} /* namespace easyhttpcpp */

