/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Path.h"

#include "easyhttpcpp/HttpCache.h"

namespace easyhttpcpp {
namespace test {

static const char* const DefaultCachePath = "./Data/HttpCache/";

class HttpCacheUnitTest : public testing::Test {
};

// path, maxSize 指定
TEST(HttpCacheUnitTest, create_createsHttpCacheWithPathAndMaxSize_WhenSpecifiedParameter)
{
    // Given: specified path and maxSize
    Poco::Path path(DefaultCachePath);
    size_t maxSize = 100;

    // When: call createCache
    HttpCache::Ptr pCache = HttpCache::createCache(path, maxSize);

    // Then: getPath get specified path and maxSize
    const Poco::Path& gottenPath = pCache->getPath();
    EXPECT_EQ(path.toString(), gottenPath.toString());
    EXPECT_EQ(maxSize, pCache->getMaxSize());
}

} /* namespace test */
} /* namespace easyhttpcpp */
