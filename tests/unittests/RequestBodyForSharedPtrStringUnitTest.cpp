/*
 * Copyright 2018 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Exception.h"

#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/RequestBody.h"

#include "RequestBodyForSharedPtrString.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";

TEST(RequestBodyForSharedPtrStringUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::string> pContent = new std::string(Content);

    // When: call RequestBodyForSharedPtrString()
    RequestBodyForSharedPtrString requestBody(pMediaType, pContent);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_EQ(pMediaType, requestBody.getMediaType());
}

TEST(RequestBodyForSharedPtrStringUnitTest, constructor_ReturnsInstance_WhenMediaTypeIsNull)
{
    // Given: none
    // When: call RequestBodyForSharedPtrString()
    Poco::SharedPtr<std::string> pContent = new std::string(Content);
    RequestBodyForSharedPtrString requestBody(NULL, pContent);

    // Then: parameters are set from content
    EXPECT_TRUE(requestBody.hasContentLength());
    EXPECT_EQ(Content.size(), requestBody.getContentLength());
    EXPECT_TRUE(requestBody.getMediaType().isNull());
}

TEST(RequestBodyForSharedPtrStringUnitTest, writeTo_WritesToOutputStream)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    Poco::SharedPtr<std::string> pContent = new std::string(Content);
    RequestBodyForSharedPtrString requestBody(pMediaType, pContent);

    // When: call writeTo()
    std::ostringstream os;
    requestBody.writeTo(os);

    // Then: parameters are set from content
    EXPECT_EQ(Content, os.str());
}

} /* namespace test */
} /* namespace easyhttpcpp */

