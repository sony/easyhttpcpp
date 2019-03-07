/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/RequestBody.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string ContentTypeHtml = "text/html";
static const std::string Content = "test content data";

TEST(RequestBodyUnitTest, createWithIStream_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::istream> pContent = new std::istringstream(Content);

    // When: call create()
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, pContent);

    // Then: parameters are set from content
    EXPECT_FALSE(pRequestBody.isNull());
    EXPECT_FALSE(pRequestBody->hasContentLength());
    EXPECT_EQ(pMediaType, pRequestBody->getMediaType());
}

TEST(RequestBodyUnitTest, createWithString_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));

    // When: call create()
    Poco::SharedPtr<std::string> pContent = new std::string(Content);
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, pContent);

    // Then: parameters are set from content
    EXPECT_FALSE(pRequestBody.isNull());
    EXPECT_TRUE(pRequestBody->hasContentLength());
    EXPECT_EQ(Content.size(), pRequestBody->getContentLength());
    EXPECT_EQ(pMediaType, pRequestBody->getMediaType());
}

TEST(RequestBodyUnitTest, createWithByteArrayBuffer_ReturnsInstance)
{
    // Given: none
    Poco::SharedPtr<ByteArrayBuffer> pBuffer = new ByteArrayBuffer(Content);
    MediaType::Ptr pMediaType(new MediaType(ContentType));

    // When: call create()
    RequestBody::Ptr pRequestBody = RequestBody::create(pMediaType, pBuffer);

    // Then: parameters are set from content
    EXPECT_FALSE(pRequestBody.isNull());
    EXPECT_TRUE(pRequestBody->hasContentLength());
    EXPECT_EQ(Content.size(), pRequestBody->getContentLength());
    EXPECT_EQ(pMediaType, pRequestBody->getMediaType());
}

} /* namespace test */
} /* namespace easyhttpcpp */

