/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/CacheInfoWithDataSize.h"

namespace easyhttpcpp {
namespace common {
namespace test {

static const char* const Key1 = "key1";
static const char* const Key2 = "key2";
static const size_t DataSize1 = 100;
static const size_t DataSize2 = 200;

class CacheInfoWithDataSizeUnitTest : public testing::Test {
};

TEST(CacheInfoWithDataSizeUnitTest, constructor_SetsKey_WhenSpecifiedKey)
{
    // Given: none
    // When: create CacheInfoWithDataSize with Key
    CacheInfoWithDataSize cacheInfo(Key1);

    // Then: set Key
    EXPECT_EQ(Key1, cacheInfo.getKey());
    EXPECT_EQ(0, cacheInfo.getDataSize());
}

TEST(CacheInfoWithDataSizeUnitTest, constructor_SetsKeyAndDataSize_WhenSpecifiedKeyAndDataSize)
{
    // Given: none
    // When: create CacheInfoWithDataSize with Key and dataSize
    CacheInfoWithDataSize cacheInfo(Key1, DataSize1);

    // Then: set Key and dataSize
    EXPECT_EQ(Key1, cacheInfo.getKey());
    EXPECT_EQ(DataSize1, cacheInfo.getDataSize());
}

TEST(CacheInfoWithDataSizeUnitTest, constructor_CreateByOriginal_WhenUseCacheInfoWithDataSizeParameter)
{
    // Given: create CacheInfoWithDataSize with Key and dataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);

    // When: create with CacheInfoWithDataSize parameter
    CacheInfoWithDataSize cacheInfo2(cacheInfo1);

    // Then: be copied Key and dataSize
    EXPECT_EQ(Key1, cacheInfo2.getKey());
    EXPECT_EQ(DataSize1, cacheInfo2.getDataSize());
}

TEST(CacheInfoWithDataSizeUnitTest, setKey_SetsKey)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo(Key1);

    // When: setKey
    cacheInfo.setKey(Key2);

    // Then: set Key
    EXPECT_EQ(Key2, cacheInfo.getKey());
}

TEST(CacheInfoWithDataSizeUnitTest, setDataSize_SetsDataSize)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo(Key1);

    // When: setDataSize
    cacheInfo.setDataSize(DataSize1);

    // Then: set dataSize
    EXPECT_EQ(DataSize1, cacheInfo.getDataSize());
}

TEST(CacheInfoWithDataSizeUnitTest, operatorEqual_ReturnsTrue_WhenSameParameter)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);
    CacheInfoWithDataSize cacheInfo2(Key1, DataSize1);

    // When: compare
    // Then: return true
    EXPECT_TRUE(cacheInfo1 == cacheInfo2);
}

TEST(CacheInfoWithDataSizeUnitTest, operatorEqual_ReturnsTrue_WhenSameInstance)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);

    // When: compare
    // Then: return true
    EXPECT_TRUE(cacheInfo1 == cacheInfo1);
}

TEST(CacheInfoWithDataSizeUnitTest, operatorEqual_ReturnsFalse_WhenDifferentKey)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);
    CacheInfoWithDataSize cacheInfo2(Key2, DataSize1);

    // When: compare
    // Then: return true
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(CacheInfoWithDataSizeUnitTest, operatorEqual_ReturnsFalse_WhenDifferentData)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);
    CacheInfoWithDataSize cacheInfo2(Key1, DataSize2);

    // When: compare
    // Then: return true
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(CacheInfoWithDataSizeUnitTest, assignmentOperator_assignsOriginalData_WhenSpecifyOriginalData)
{
    // Given: create CacheInfoWithDataSize
    CacheInfoWithDataSize cacheInfo1(Key1, DataSize1);
    CacheInfoWithDataSize cacheInfo2(Key1, DataSize2);

    // When: assignment operator
    cacheInfo1 = cacheInfo2;

    // Then: be copied original data
    EXPECT_TRUE(cacheInfo1 == cacheInfo2);
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
