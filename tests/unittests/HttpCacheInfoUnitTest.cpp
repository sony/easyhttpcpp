/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "HttpCacheInfo.h"

namespace easyhttpcpp {
namespace test {

static const char* const Key1 = "key1";
static const char* const Key2 = "key2";
static const size_t DataSize1 = 100;
static const size_t DataSize2 = 200;

class HttpCacheInfoUnitTest : public testing::Test {
};

TEST(HttpCacheInfoUnitTest, constructor_SetsKey_WhenSpecifiedKey)
{
    // When: create HttpCacheInfo with Key
    HttpCacheInfo cacheInfo(Key1);

    // Then: set Key
    EXPECT_EQ(Key1, cacheInfo.getKey());
    EXPECT_EQ(0, cacheInfo.getDataSize());
}

TEST(HttpCacheInfoUnitTest, constructor_SetsKeyAndDataSize_WhenSpecifiedKeyAndDataSize)
{
    // When: create HttpCacheInfo with Key and dataSize
    HttpCacheInfo cacheInfo(Key1, DataSize1);

    // Then: set Key and dataSize
    EXPECT_EQ(Key1, cacheInfo.getKey());
    EXPECT_EQ(DataSize1, cacheInfo.getDataSize());
}

TEST(HttpCacheInfoUnitTest, copyConstructor_CopiesAllProperties_WhenUseCacheInfoWithDataSizeParameter)
{
    // Given: create HttpCacheInfo with Key and dataSize and set reservedRemove and addDataRef
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    cacheInfo1.setReservedRemove(true);
    cacheInfo1.addDataRef();

    // When: create with CacheInfoWithDataSize parameter
    HttpCacheInfo cacheInfo2(cacheInfo1);

    // Then: be copied parameter
    EXPECT_EQ(Key1, cacheInfo2.getKey());
    EXPECT_EQ(DataSize1, cacheInfo2.getDataSize());
    EXPECT_TRUE(cacheInfo2.isReservedRemove());
    EXPECT_EQ(1, cacheInfo2.getDataRefCount());
}

TEST(HttpCacheInfoUnitTest, setReservedRemove_SetsReservedRemove_WhenSpecified)
{
    // Given: create HttpCacheInfo
    HttpCacheInfo cacheInfo(Key1);

    // When: setReservedRemove true and false
    // Then: set reservedRemove
    cacheInfo.setReservedRemove(true);
    EXPECT_TRUE(cacheInfo.isReservedRemove());
    cacheInfo.setReservedRemove(false);
    EXPECT_FALSE(cacheInfo.isReservedRemove());
}

TEST(HttpCacheInfoUnitTest, addDataRefAndReleaseDataRef_IncrementAndDecrementDataRefCont)
{
    // Given: create HttpCacheInfo
    HttpCacheInfo cacheInfo(Key1);

    // When: addDataRef and releaseDataRef
    // Then: increment and delcrement dataRefCountf

    // 0 -> 1
    cacheInfo.addDataRef();
    EXPECT_EQ(1, cacheInfo.getDataRefCount());

    // 1 -> 2
    cacheInfo.addDataRef();
    EXPECT_EQ(2, cacheInfo.getDataRefCount());

    // 2 -> 1
    cacheInfo.releaseDataRef();
    EXPECT_EQ(1, cacheInfo.getDataRefCount());

    // 1 -> 0
    cacheInfo.releaseDataRef();
    EXPECT_EQ(0, cacheInfo.getDataRefCount());

    // 0 -> 0
    cacheInfo.releaseDataRef();
    EXPECT_EQ(0, cacheInfo.getDataRefCount());
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenSameParameter)
{
    // Given: create HttpCacheInfo
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    cacheInfo1.setReservedRemove(true);
    cacheInfo1.addDataRef();
    HttpCacheInfo cacheInfo2(Key1, DataSize1);
    cacheInfo2.setReservedRemove(true);
    cacheInfo2.addDataRef();

    // When: compare
    // Then return true
    EXPECT_TRUE(cacheInfo1 == cacheInfo2);
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenSameInstance)
{
    // Given: create HttpCacheInfo
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    cacheInfo1.setReservedRemove(true);
    cacheInfo1.addDataRef();

    // When: compare
    // Then return true
    EXPECT_TRUE(cacheInfo1 == cacheInfo1);
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenDifferentKey)
{
    // Given: create CacheInfoWithDataSize
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    HttpCacheInfo cacheInfo2(Key2, DataSize1);

    // When: compare
    // Then return false
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenDifferentDataSize)
{
    // Given: create CacheInfoWithDataSize
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    HttpCacheInfo cacheInfo2(Key1, DataSize2);

    // When: compare
    // Then return false
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenDifferentReservedRemove)
{
    // Given: create CacheInfoWithDataSize
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    cacheInfo1.setReservedRemove(true);
    HttpCacheInfo cacheInfo2(Key1, DataSize1);

    // When: compare
    // Then return false
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(HttpCacheInfoUnitTest, operatorEqualTo_ReturnsTrue_WhenDifferentDataRefCount)
{
    // Given: create CacheInfoWithDataSize
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    cacheInfo1.addDataRef();
    HttpCacheInfo cacheInfo2(Key1, DataSize1);

    // When: compare
    // Then return false
    EXPECT_FALSE(cacheInfo1 == cacheInfo2);
}

TEST(HttpCacheInfoUnitTest, assignmentOperator_assignsOriginalData_WhenSpecifyOriginalData)
{
    // Given: create CacheInfoWithDataSize
    HttpCacheInfo cacheInfo1(Key1, DataSize1);
    HttpCacheInfo cacheInfo2(Key2, DataSize2);
    cacheInfo2.setReservedRemove(true);
    cacheInfo2.addDataRef();

    // When: assignment operator
    cacheInfo1 = cacheInfo2;

    // Then: be copied original data
    EXPECT_TRUE(cacheInfo1 == cacheInfo2);
}

} /* namespace test */
} /* namespace easyhttpcpp */
