/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/RequestBody.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForString.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";

TEST(RequestBodyForStringUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));

    // When: call RequestBodyForString()
    RequestBodyForString requestBody(pMediaType, Content);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForStringUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    // When: call RequestBodyForString()
    RequestBodyForString requestBody(NULL, Content);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForStringUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    RequestBodyForString requestBody(pMediaType, Content);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

} /* namespace test */
} /* namespace easyhttpcpp */

