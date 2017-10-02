/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/MediaType.h"

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";

TEST(MediaTypeUnitTest, toString_ReturnsContentType)
{
    // Given: none
    MediaType mediaType(ContentType);
    // When: call toString()
    // Then: returns Content-Type
    EXPECT_EQ(ContentType, mediaType.toString());
}

} /* namespace test */
} /* namespace easyhttpcpp */

