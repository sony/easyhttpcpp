/*
 * Copyright 2017 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "easyhttpcpp/common/CacheStrategyListener.h"
#include "easyhttpcpp/common/LruCacheByDataSizeStrategy.h"
#include "MockCacheStrategyListener.h"

using easyhttpcpp::testutil::MockCacheStrategyListener;

namespace easyhttpcpp {
namespace common {
namespace test {

static const size_t DefaultMaxSise = 500;
static const char* const Key1 = "key1";
static const char* const Key2 = "key2";
static const char* const Key3 = "key3";
static const char* const Key4 = "key4";
static const char* const Key5 = "key5";
static const char* const Key6 = "key6";
static const size_t DataSize1 = 100;
static const size_t DataSize2 = 150;

namespace {

bool isCacheInfoKey1AndDataSize100(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    return (pCacheInfo->getKey() == Key1 && pCacheInfo->getDataSize() == 100);
}

bool isCacheInfoKey1AndDataSize1(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    return (pCacheInfo->getKey() == Key1 && pCacheInfo->getDataSize() == DataSize1);
}

} /* namespace */

class LruCacheByDataSizeStrategyUnitTest : public testing::Test {
protected:
    MockCacheStrategyListener m_mockCacheStrategyListener;
};


// add
TEST_F(LruCacheByDataSizeStrategyUnitTest, add_ReturnsFalse_WhenFreeSpaceAndListenerReturnsFalse)
{
    CacheInfoWithDataSize::Ptr pCacheInfo = new CacheInfoWithDataSize(Key1, DataSize1);
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key1, testing::Eq(testing::ByRef(pCacheInfo)))).
            WillOnce(testing::Return(false));

    // Given: setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    // When: call add and listener return false
    // Then: return false and do not add to list
    EXPECT_FALSE(cacheStrategy.add(Key1, pCacheInfo));

    // totalSize
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_TRUE(pGottenCacheInfo.isNull());
}

// add
TEST_F(LruCacheByDataSizeStrategyUnitTest, add_ReturnsTrue_WhenFreeSpaceAndListenerReturnsTrue)
{
    CacheInfoWithDataSize::Ptr pCacheInfo = new CacheInfoWithDataSize(Key1, DataSize1);
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key1, testing::Eq(testing::ByRef(pCacheInfo)))).
            WillOnce(testing::Return(true));

    // Given: setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    // When: call add and listener return true
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key1, pCacheInfo));

    // totalSize
    EXPECT_EQ(DataSize1, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo, *pGottenCacheInfo);
}

// add
TEST_F(LruCacheByDataSizeStrategyUnitTest, add_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvict)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfo4 = new CacheInfoWithDataSize(Key4, 50);
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::Eq(testing::ByRef(pCacheInfo4)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));

    // When: call add and listener return true
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key4, pCacheInfo4));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key4);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo4, *pGottenCacheInfo);
}

// add
TEST_F(LruCacheByDataSizeStrategyUnitTest, add_ReturnsFalse_WhenNoFreeSpaceAndCanNotMakeFreeSpaceByEvict)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfo4 = new CacheInfoWithDataSize(Key4, 350);
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::Eq(testing::ByRef(pCacheInfo4)))).
            WillOnce(testing::Return(true));

    // When: call add and listener return true
    // Then: return false and do not add
    EXPECT_FALSE(cacheStrategy.add(Key4, pCacheInfo4));

    // totalSize
    EXPECT_EQ(300, cacheStrategy.getTotalSize());
}

// add
TEST_F(LruCacheByDataSizeStrategyUnitTest, add_ReturnsTrue_WhenFreeSpaceAndNoSetListener)
{
    CacheInfoWithDataSize::Ptr pCacheInfo = new CacheInfoWithDataSize(Key1, DataSize1);

    // Given: do not call setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);

    // When: call add
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key1, pCacheInfo));

    // totalSize
    EXPECT_EQ(DataSize1, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsFalse_WhenFreeSpaceAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key1, DataSize2);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key1, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(false));

    // When: call update and listener return true
    // Then: return false and do not update
    EXPECT_FALSE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(DataSize1, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo1, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key1, DataSize2);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key1, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(DataSize2, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfoForUpdate, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvict)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key2, 150);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key2, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key2, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfoForUpdate, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvictAndKeyWillBeEvicted)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key1, 150);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key1, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfoForUpdate, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsFalse_WhenNoFreeSpaceAndCanNotMakeFreeSpaceByEvict)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key2, 450);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key2, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return false
    EXPECT_FALSE(cacheStrategy.update(Key2, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(300, cacheStrategy.getTotalSize());
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndNotExistKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key2, DataSize2);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key2, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key2, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(DataSize1 + DataSize2, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfoForUpdate, *pGottenCacheInfo);
}

// update
TEST_F(LruCacheByDataSizeStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndNoListener)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = new CacheInfoWithDataSize(Key1, DataSize2);

    // When: call update
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(DataSize2, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfoForUpdate, *pGottenCacheInfo);
}

// remove
TEST_F(LruCacheByDataSizeStrategyUnitTest, remove_ReturnsFalseAndDoesNotCallListener_WhenNotExistKey)
{
    // Given: do not add key and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).Times(0);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheStrategy.remove(Key1));
}

// remove
TEST_F(LruCacheByDataSizeStrategyUnitTest, remove_ReturnsFalse_WhenExistsKeyAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(false));

    // When: call remove and listener return false
    // Then: return false
    EXPECT_FALSE(cacheStrategy.remove(Key1));

    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
}

// remove
TEST_F(LruCacheByDataSizeStrategyUnitTest, remove_ReturnsTrue_WhenExistsKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));

    // When: call remove and listener return true
    // Then: return true and remove from list
    EXPECT_TRUE(cacheStrategy.remove(Key1));

    // totalSize
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_TRUE(pGottenCacheInfo.isNull());
}

// remove
TEST_F(LruCacheByDataSizeStrategyUnitTest, remove_ReturnsTrue_WhenExistsKeyAndNoListener)
{
    // Given: add Key1 and do not setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    // When: call remove
    // Then: return true and remove from list
    EXPECT_TRUE(cacheStrategy.remove(Key1));

    // totalSize
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_TRUE(pGottenCacheInfo.isNull());
}

// get
TEST_F(LruCacheByDataSizeStrategyUnitTest, get_ReturnsNullAndDoesNotCallListener_WhenNotExistKey)
{
    // Given: do not add key and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::_)).Times(0);

    // When: call get
    // Then: return NULL
    EXPECT_TRUE(cacheStrategy.get(Key1).isNull());
}

// get
TEST_F(LruCacheByDataSizeStrategyUnitTest, get_ReturnsNull_WhenExistsKeyAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(isCacheInfoKey1AndDataSize1))).
            WillOnce(testing::Return(false));

    // When: call get and listener return false
    // Then: return NULL
    EXPECT_TRUE(cacheStrategy.get(Key1).isNull());
}

// get
TEST_F(LruCacheByDataSizeStrategyUnitTest, get_ReturnsNotNull_WhenExistsKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, DataSize1);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(isCacheInfoKey1AndDataSize1))).
            WillOnce(testing::Return(true));

    // When: call get and listener return true
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);

    // Then: return not NULL
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo1, *pGottenCacheInfo);
}

// get
TEST_F(LruCacheByDataSizeStrategyUnitTest, get_ReturnsNotNullAndMovesNewestOfLruList_WhenGetsOldestKey)
{
    // Given: add until maxSize and setListener
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(isCacheInfoKey1AndDataSize100))).
            WillOnce(testing::Return(true));

    // When: call get and listener return true
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);

    // Then: return not NULL and move to newest of LRU list
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    EXPECT_EQ(*pCacheInfo1, *pGottenCacheInfo);

    // confirm by get
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(true));
    CacheInfoWithDataSize::Ptr pCacheInfo4 = new CacheInfoWithDataSize(Key4, 100);
    EXPECT_TRUE(cacheStrategy.add(Key4, pCacheInfo4));

    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key5, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key3)).WillOnce(testing::Return(true));
    CacheInfoWithDataSize::Ptr pCacheInfo5 = new CacheInfoWithDataSize(Key5, 100);
    EXPECT_TRUE(cacheStrategy.add(Key5, pCacheInfo5));

    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key6, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));
    CacheInfoWithDataSize::Ptr pCacheInfo6 = new CacheInfoWithDataSize(Key6, 100);
    EXPECT_TRUE(cacheStrategy.add(Key6, pCacheInfo6));
}

// clear
TEST_F(LruCacheByDataSizeStrategyUnitTest, clear_ReturnsTrue_WhenListIsEmptyAndMayDeleteIfBustIsTrue)
{
    // Given: do not add list
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(true));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(LruCacheByDataSizeStrategyUnitTest, clear_ReturnsTrueAndClearsList_WhenListIsNotEmptyAndMayDeleteIfBustIsTrue)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(true));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(LruCacheByDataSizeStrategyUnitTest, clear_ReturnsTrue_WhenListIsEmptyAndMayDeleteIfBustIsFalse)
{
    // Given: do not add list
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(false));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(LruCacheByDataSizeStrategyUnitTest, clear_ReturnsTrueAndClearsList_WhenListIsNotEmptyAndMayDeleteIfBustIsFalse)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(false));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest, makeSpace_ReturnsTrue_WhenTotalSizePlusRequestSizeLessThanMaxSize)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    ASSERT_EQ(200, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalSize + requestSize < maxSize
    // Then: return true
    EXPECT_TRUE(cacheStrategy.makeSpace(50));
    EXPECT_EQ(200, cacheStrategy.getTotalSize());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest, makeSpace_ReturnsTrue_WhenTotalSizePlusRequestSizeEqualsMaxSize)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    ASSERT_EQ(200, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalSize + requestSize == maxSize
    // Then: return true
    EXPECT_TRUE(cacheStrategy.makeSpace(100));
    EXPECT_EQ(200, cacheStrategy.getTotalSize());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest,
        makeSpace_ReturnsFalse_WhenTotalSizeIsZeroAndMaxSizeIsZeroAndRequestSizeGraterThanZero)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(0);
    ASSERT_EQ(0, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalSize == 0, maxSize == 0, requestSize == 1
    // Then: return false
    EXPECT_FALSE(cacheStrategy.makeSpace(1));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest,
        makeSpace_ReturnsTrueAndUpdateTotalSize_WhenTotalSizePlusRequestSizeGraterThanMaxSizeAndBeAbleToMakeFreeSpace)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalsize + requestSize > maxSize and be able to make free space
    // Then: return true and update totalSize
    EXPECT_TRUE(cacheStrategy.makeSpace(50));
    EXPECT_EQ(200, cacheStrategy.getTotalSize());

    // confirm by get (remove Key1)
    CacheInfoWithDataSize::Ptr pGottenCacheInfo1 = cacheStrategy.get(Key1);
    EXPECT_TRUE(pGottenCacheInfo1.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo2.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_FALSE(pGottenCacheInfo3.isNull());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest,
        makeSpace_ReturnsTrueAndUpdateTotalSize_WhenTotalSizePlusRequestSizeGraterThanMaxSizeAndBeAbleToMakeFreeSpaceAfterGet)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    cacheStrategy.get(Key1);
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalsize + requestSize > maxSize and be able to make free space
    // Then: return true and update totalSize
    EXPECT_TRUE(cacheStrategy.makeSpace(50));
    EXPECT_EQ(200, cacheStrategy.getTotalSize());

    // confirm by get (remove Key2)
    CacheInfoWithDataSize::Ptr pGottenCacheInfo1 = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo1.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_TRUE(pGottenCacheInfo2.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_FALSE(pGottenCacheInfo3.isNull());
}

// makeSpace
TEST_F(LruCacheByDataSizeStrategyUnitTest,
        makeSpace_ReturnsFalse_WhenTotalSizePlusRequestSizeGraterThanMaxSizeAndCanNotMakeFreeSpace)
{
    // Given: add to list
    LruCacheByDataSizeStrategy cacheStrategy(300);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    CacheInfoWithDataSize::Ptr pCacheInfo2 = new CacheInfoWithDataSize(Key2, 100);
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    CacheInfoWithDataSize::Ptr pCacheInfo3 = new CacheInfoWithDataSize(Key3, 100);
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call makeSpace : totalsize + requestSize > maxSize and can not make free space
    // Then: return true and update totalSize
    EXPECT_FALSE(cacheStrategy.makeSpace(350));
    EXPECT_EQ(300, cacheStrategy.getTotalSize());
}

// getMaxSize
TEST_F(LruCacheByDataSizeStrategyUnitTest, getMaxSize_ReturnsMaxSize)
{
    // Given: create LruCacheByDataSizeStrategy by specified max size.
    LruCacheByDataSizeStrategy cacheStrategy(300);

    // When: call getMaxSize
    // Then: return specified max size
    EXPECT_EQ(300, cacheStrategy.getMaxSize());
}

// getTotalSize
TEST_F(LruCacheByDataSizeStrategyUnitTest, getTotalSize_ReturnsZero_WhenAfterCreateLruCacheByDataSizeStrategy)
{
    // Given: create LruCacheByDataSizeStrategy
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);

    // When: call getTotalSize
    // Then: return 0
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// isEmpty
TEST_F(LruCacheByDataSizeStrategyUnitTest, isEmpty_ReturnsFalse_WhenAfterAdd)
{
    // Given: create LruCacheByDataSizeStrategy and add cacheInfo
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = new CacheInfoWithDataSize(Key1, 100);
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    // When: call isEmpty
    // Then: return false
    EXPECT_FALSE(cacheStrategy.isEmpty());
}

// isEmpty
TEST_F(LruCacheByDataSizeStrategyUnitTest, isEmpty_ReturnsTrue_WhenAfterCreateLruCacheByDataSizeStrategy)
{
    // Given: create LruCacheByDataSizeStrategy
    LruCacheByDataSizeStrategy cacheStrategy(DefaultMaxSise);

    // When: call isEmpty
    // Then: return true
    EXPECT_TRUE(cacheStrategy.isEmpty());
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
