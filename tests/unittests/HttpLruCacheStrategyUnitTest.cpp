/*
 * Copyright 2017 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "easyhttpcpp/common/CacheInfoWithDataSize.h"
#include "easyhttpcpp/common/CacheStrategyListener.h"
#include "MockCacheStrategyListener.h"

#include "HttpCacheInfo.h"
#include "HttpLruCacheStrategy.h"

using easyhttpcpp::common::CacheInfoWithDataSize;
using easyhttpcpp::common::CacheStrategyListener;
using easyhttpcpp::testutil::MockCacheStrategyListener;

namespace easyhttpcpp {
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

class HttpLruCacheStrategyUnitTest : public testing::Test {
protected:
    // Objects declared here can be used by all tests in the test case for HttpLruCacheStrategyUnitTest.
    MockCacheStrategyListener m_mockCacheStrategyListener;
};

static bool CheckHttpCacheInfoKey1AndDataSize1(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*>(pCacheInfo.get());
    return (pHttpCacheInfo->getKey() == Key1 && pHttpCacheInfo->getDataSize() == DataSize1 &&
            !pHttpCacheInfo->isReservedRemove() && pHttpCacheInfo->getDataRefCount() == 0);
}

static bool CheckHttpCacheInfoKey1AndDataSize100(CacheInfoWithDataSize::Ptr pCacheInfo)
{
    HttpCacheInfo* pHttpCacheInfo = static_cast<HttpCacheInfo*>(pCacheInfo.get());
    return (pHttpCacheInfo->getKey() == Key1 && pHttpCacheInfo->getDataSize() == 100 &&
            !pHttpCacheInfo->isReservedRemove() && pHttpCacheInfo->getDataRefCount() == 0);
}


// add
TEST_F(HttpLruCacheStrategyUnitTest, add_ReturnsFalse_WhenFreeSpaceAndListenerReturnsFalse)
{
    HttpCacheInfo* pHttpCacheInfo = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo = pHttpCacheInfo;
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key1, testing::Eq(testing::ByRef(pCacheInfo)))).
            WillOnce(testing::Return(false));

    // Given: setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
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
TEST_F(HttpLruCacheStrategyUnitTest, add_ReturnsTrue_WhenFreeSpaceAndListenerReturnsTrue)
{
    HttpCacheInfo* pHttpCacheInfo = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo = pHttpCacheInfo;
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key1, testing::Eq(testing::ByRef(pCacheInfo)))).
            WillOnce(testing::Return(true));

    // Given: setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
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
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo, *pHttpGottenCacheInfo);
}

// add
TEST_F(HttpLruCacheStrategyUnitTest, add_ReturnsTrueAndReplacesCacheInfo_WhenFreeSpaceAndListenerReturnsTrueAndExistKey)
{
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key1, DataSize2);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key1, testing::_)).
            WillRepeatedly(testing::Return(true));

    // Given: setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    // add
    EXPECT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    EXPECT_EQ(DataSize1, cacheStrategy.getTotalSize());

    // When: call add and listener return true
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key1, pCacheInfo2));

    // totalSize
    EXPECT_EQ(DataSize2, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo2, *pHttpGottenCacheInfo);
}

// add
TEST_F(HttpLruCacheStrategyUnitTest,
        add_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvictWithExistsDataRefCountGraterThanZero)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfo4 = new HttpCacheInfo(Key4, 50);
    CacheInfoWithDataSize::Ptr pCacheInfo4 = pHttpCacheInfo4;
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::Eq(testing::ByRef(pCacheInfo4)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(true));

    // When: call add and listener return true
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key4, pCacheInfo4));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo4 = cacheStrategy.get(Key4);
    EXPECT_FALSE(pGottenCacheInfo4.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo4 = static_cast<HttpCacheInfo*>(pGottenCacheInfo4.get());
    EXPECT_EQ(*pHttpCacheInfo4, *pHttpGottenCacheInfo4);
    // check remove Key2, do nt remove Key1
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_TRUE(pGottenCacheInfo2.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo1 = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo1.isNull());
}

// add
TEST_F(HttpLruCacheStrategyUnitTest,
        add_ReturnsFalse_WhenNoFreeSpaceAndCanNotMakeFreeSpaceByEvictWithExistsDataRefCountGraterThanZero)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    pHttpCacheInfo2->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfo4 = new HttpCacheInfo(Key4, 150);
    CacheInfoWithDataSize::Ptr pCacheInfo4 = pHttpCacheInfo4;
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::Eq(testing::ByRef(pCacheInfo4)))).
            WillOnce(testing::Return(true));

    // When: call add and listener return true
    // Then: return false and do not add
    EXPECT_FALSE(cacheStrategy.add(Key4, pCacheInfo4));

    // totalSize
    EXPECT_EQ(300, cacheStrategy.getTotalSize());
}

// add
TEST_F(HttpLruCacheStrategyUnitTest, add_ReturnsTrue_WhenFreeSpaceAndNoSetListener)
{
    HttpCacheInfo* pHttpCacheInfo = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo = pHttpCacheInfo;

    // Given: do not call setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);

    // When: call add
    // Then: return true and add to list
    EXPECT_TRUE(cacheStrategy.add(Key1, pCacheInfo));

    // totalSize
    EXPECT_EQ(DataSize1, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo, *pHttpGottenCacheInfo);
}

// update
TEST_F(HttpLruCacheStrategyUnitTest, update_ReturnsFalse_WhenFreeSpaceAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key1, DataSize2);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
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
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo1, *pHttpGottenCacheInfo);
}

// update
TEST_F(HttpLruCacheStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key1, DataSize2);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
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
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfoForUpdate, *pHttpGottenCacheInfo);
}

// update
TEST_F(HttpLruCacheStrategyUnitTest,
        update_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvictWithExistsDataRefCountGreaterThanZero)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key3, 150);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key3, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key3, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_FALSE(pGottenCacheInfo3.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo3 = static_cast<HttpCacheInfo*>(pGottenCacheInfo3.get());
    EXPECT_EQ(*pCacheInfoForUpdate, *pHttpGottenCacheInfo3);
    // check remove Key2
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_TRUE(pGottenCacheInfo2.isNull());
}

// update
TEST_F(HttpLruCacheStrategyUnitTest,
        update_ReturnsTrue_WhenNoFreeSpaceAndMakeFreeSpaceByEvictWithExistsDataRefCountGraterThanZeroAndKeyWillBeEvicted)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key2, 150);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key2, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key3)).WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key2, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(250, cacheStrategy.getTotalSize());
    // confirm by get
    cacheStrategy.setListener(NULL);
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo2.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo2 = static_cast<HttpCacheInfo*>(pGottenCacheInfo2.get());
    EXPECT_EQ(*pCacheInfoForUpdate, *pHttpGottenCacheInfo2);
    // check remove Key3
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_TRUE(pGottenCacheInfo3.isNull());
}

// update
TEST_F(HttpLruCacheStrategyUnitTest,
        update_ReturnsFalse_WhenNoFreeSpaceAndCanNotMakeFreeSpaceByEvictWithExistsDataRefCountGraterThanZero)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    pHttpCacheInfo3->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key2, 150);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key2, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return false
    EXPECT_FALSE(cacheStrategy.update(Key2, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(300, cacheStrategy.getTotalSize());
}

// update
TEST_F(HttpLruCacheStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndNotExistKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key2, DataSize2);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
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
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfoForUpdate, *pHttpGottenCacheInfo);
}

// update
TEST_F(HttpLruCacheStrategyUnitTest, update_ReturnsTrue_WhenFreeSpaceAndNoListener)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key1, DataSize2);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;

    // When: call update
    // Then: return true and update list
    EXPECT_TRUE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(DataSize2, cacheStrategy.getTotalSize());
    // confirm by get
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfoForUpdate, *pHttpGottenCacheInfo);
}

// update
TEST_F(HttpLruCacheStrategyUnitTest,
        update_ReturnsTrueAndRemove_WhenExistKeyAndDataRefCountIsOneAndReservedRemoveIsTrue)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    pHttpCacheInfo1->setReservedRemove(true);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    HttpCacheInfo* pHttpCacheInfoForUpdate = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfoForUpdate->setReservedRemove(true);
    CacheInfoWithDataSize::Ptr pCacheInfoForUpdate = pHttpCacheInfoForUpdate;
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key1, testing::Eq(testing::ByRef(pCacheInfoForUpdate)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));

    // When: call update and listener return true
    // Then: return true
    EXPECT_TRUE(cacheStrategy.update(Key1, pCacheInfoForUpdate));

    // totalSize
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// remove
TEST_F(HttpLruCacheStrategyUnitTest, remove_ReturnsFalseAndDoesNotCallListener_WhenNotExistKey)
{
    // Given: do not add key and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(testing::_)).Times(0);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheStrategy.remove(Key1));
}

// remove
TEST_F(HttpLruCacheStrategyUnitTest, remove_ReturnsFalse_WhenExistsKeyAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
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
TEST_F(HttpLruCacheStrategyUnitTest, remove_ReturnsTrue_WhenExistsKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
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
TEST_F(HttpLruCacheStrategyUnitTest, remove_ReturnsTrue_WhenExistsKeyAndNoListener)
{
    // Given: add Key1 and do not setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
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

// remove
TEST_F(HttpLruCacheStrategyUnitTest,
        remove_ReturnsTrueAndReservedRemove_WhenExistKeyAndDataRefCountIsOne)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).Times(0);
    EXPECT_CALL(m_mockCacheStrategyListener, onUpdate(Key1, testing::_)).WillOnce(testing::Return(true));

    // When: call remove and listener return true
    // Then: return true and set reservedRemove flag
    EXPECT_TRUE(cacheStrategy.remove(Key1));

    cacheStrategy.setListener(NULL);
    // check resercedRemove flag
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(1, pHttpGottenCacheInfo->getDataRefCount());
    EXPECT_TRUE(pHttpGottenCacheInfo->isReservedRemove());
}

// get
TEST_F(HttpLruCacheStrategyUnitTest, get_ReturnsNullAndDoesNotCallListener_WhenNotExistKey)
{
    // Given: do not add key and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::_)).Times(0);

    // When: call get
    // Then: return NULL
    EXPECT_TRUE(cacheStrategy.get(Key1).isNull());
}

// get
TEST_F(HttpLruCacheStrategyUnitTest, get_ReturnsNull_WhenExistsKeyAndListenerReturnsFalse)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(CheckHttpCacheInfoKey1AndDataSize1))).
            WillOnce(testing::Return(false));

    // When: call get and listener return false
    // Then: return NULL
    EXPECT_TRUE(cacheStrategy.get(Key1).isNull());
}

// get
TEST_F(HttpLruCacheStrategyUnitTest, get_ReturnsNotNull_WhenExistsKeyAndListenerReturnsTrue)
{
    // Given: add Key1 and setListener
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, DataSize1);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));

    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(CheckHttpCacheInfoKey1AndDataSize1))).
            WillOnce(testing::Return(true));

    // When: call get and listener return true
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);

    // Then: return not NULL
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo1, *pHttpGottenCacheInfo);
    // address is not equal (get create new instance)
    EXPECT_NE(pHttpCacheInfo1, pHttpGottenCacheInfo);
}

// get
TEST_F(HttpLruCacheStrategyUnitTest, get_ReturnsNotNullAndMovesNewestOfLruList_WhenGetsOldestKey)
{
    // Given: add until maxSize and setListener
    HttpLruCacheStrategy cacheStrategy(300);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());
    cacheStrategy.setListener(&m_mockCacheStrategyListener);

    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key1, testing::Truly(CheckHttpCacheInfoKey1AndDataSize100))).
            WillOnce(testing::Return(true));

    // When: call get and listener return true
    CacheInfoWithDataSize::Ptr pGottenCacheInfo = cacheStrategy.get(Key1);

    // Then: return not NULL and move to newest of LRU list
    EXPECT_FALSE(pGottenCacheInfo.isNull());
    HttpCacheInfo* pHttpGottenCacheInfo = static_cast<HttpCacheInfo*>(pGottenCacheInfo.get());
    EXPECT_EQ(*pHttpCacheInfo1, *pHttpGottenCacheInfo);

    // confirm by get
    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key4, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(true));
    HttpCacheInfo* pHttpCacheInfo4 = new HttpCacheInfo(Key4, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo4 = pHttpCacheInfo4;
    EXPECT_TRUE(cacheStrategy.add(Key4, pCacheInfo4));

    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key5, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key3)).WillOnce(testing::Return(true));
    HttpCacheInfo* pHttpCacheInfo5 = new HttpCacheInfo(Key5, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo5 = pHttpCacheInfo5;
    EXPECT_TRUE(cacheStrategy.add(Key5, pCacheInfo5));

    EXPECT_CALL(m_mockCacheStrategyListener, onAdd(Key6, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));
    HttpCacheInfo* pHttpCacheInfo6 = new HttpCacheInfo(Key6, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo6 = pHttpCacheInfo6;
    EXPECT_TRUE(cacheStrategy.add(Key6, pCacheInfo6));
}

// clear
TEST_F(HttpLruCacheStrategyUnitTest, clear_ReturnsTrue_WhenListIsEmptyAndMayDeleteIfBustIsTrue)
{
    // Given: do not add list
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(true));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(HttpLruCacheStrategyUnitTest,
        clear_ReturnsTrueAndClearsList_WhenListIsNotEmptyAndMayDeleteIfBustIsTrueAndExistsDataRefCountGraterThanZero)
{
    // Given: add to list
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    pHttpCacheInfo2->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(true));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(HttpLruCacheStrategyUnitTest, clear_ReturnsTrue_WhenListIsEmptyAndMayDeleteIfBustIsFalse)
{
    // Given: do not add list
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);

    // When: call clear
    // Then: return true and list is empty
    EXPECT_TRUE(cacheStrategy.clear(false));
    EXPECT_EQ(0, cacheStrategy.getTotalSize());
}

// clear
TEST_F(HttpLruCacheStrategyUnitTest,
        clear_ReturnsTrueAndClearsListExceptDataRefContGratherThanZero_WhenListIsNotEmptyAndMayDeleteIfBustIsFalseExistsDataRefCountGraterThanZero)
{
    // Given: add to list
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    pHttpCacheInfo2->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    // When: call clear
    // Then: return false
    EXPECT_FALSE(cacheStrategy.clear(false));

    // check cleared key
    EXPECT_EQ(200, cacheStrategy.getTotalSize());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo1 = cacheStrategy.get(Key1);
    EXPECT_FALSE(pGottenCacheInfo1.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo2.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_TRUE(pGottenCacheInfo3.isNull());
}

// clear
TEST_F(HttpLruCacheStrategyUnitTest, clear_ReturnsFalseAndOtherThanFailureRemovesFromList_WhenOnRemoveReturnedFalse)
{
    // Given: add to list and onRemove of Key2 return false.
    HttpLruCacheStrategy cacheStrategy(DefaultMaxSise);
    HttpCacheInfo* pHttpCacheInfo1 = new HttpCacheInfo(Key1, 100);
    pHttpCacheInfo1->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo1 = pHttpCacheInfo1;
    ASSERT_TRUE(cacheStrategy.add(Key1, pCacheInfo1));
    HttpCacheInfo* pHttpCacheInfo2 = new HttpCacheInfo(Key2, 100);
    pHttpCacheInfo2->addDataRef();
    CacheInfoWithDataSize::Ptr pCacheInfo2 = pHttpCacheInfo2;
    ASSERT_TRUE(cacheStrategy.add(Key2, pCacheInfo2));
    HttpCacheInfo* pHttpCacheInfo3 = new HttpCacheInfo(Key3, 100);
    CacheInfoWithDataSize::Ptr pCacheInfo3 = pHttpCacheInfo3;
    ASSERT_TRUE(cacheStrategy.add(Key3, pCacheInfo3));
    ASSERT_EQ(300, cacheStrategy.getTotalSize());

    cacheStrategy.setListener(&m_mockCacheStrategyListener);
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key1)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key2)).WillOnce(testing::Return(false));
    EXPECT_CALL(m_mockCacheStrategyListener, onRemove(Key3)).WillOnce(testing::Return(true));
    EXPECT_CALL(m_mockCacheStrategyListener, onGet(Key2, testing::_)).WillOnce(testing::Return(true));

    // When: call clear
    // Then: return false and key2 だけが残る。
    EXPECT_FALSE(cacheStrategy.clear(true));

    CacheInfoWithDataSize::Ptr pGottenCacheInfo1 = cacheStrategy.get(Key1);
    EXPECT_TRUE(pGottenCacheInfo1.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo2 = cacheStrategy.get(Key2);
    EXPECT_FALSE(pGottenCacheInfo2.isNull());
    CacheInfoWithDataSize::Ptr pGottenCacheInfo3 = cacheStrategy.get(Key3);
    EXPECT_TRUE(pGottenCacheInfo3.isNull());
}

} /* namespace test */
} /* namespace easyhttpcpp */
