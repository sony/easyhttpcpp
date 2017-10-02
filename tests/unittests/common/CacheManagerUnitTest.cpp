/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/CacheManager.h"
#include "MockCache.h"

using easyhttpcpp::common::ByteArrayBuffer;
using easyhttpcpp::testutil::MockCache;

namespace easyhttpcpp {
namespace common {
namespace test {

static const std::string Key1 = "key1";
static const std::string Path1 = "tmp/file.dat";
static const std::string Data1 = "test_data";
static std::istream* Stream1 = reinterpret_cast<std::istream*>(0x1234);

class CacheManagerUnitTest : public testing::Test {
protected:

    // Objects declared here can be used by all tests in the test case for CacheManagerUnitTest.
    MockCache::Ptr m_pMockL1Cache;
    MockCache::Ptr m_pMockL2Cache;

    void SetUp()
    {
        m_pMockL1Cache = new MockCache();
        m_pMockL2Cache = new MockCache();
    }

    void TearDown()
    {
        m_pMockL1Cache = NULL;
        m_pMockL2Cache = NULL;
    }

    CacheMetadata::Ptr setupCacheMetadata()
    {
        CacheMetadata::Ptr pCacheMetadata(new CacheMetadata());
        pCacheMetadata->setKey(Key1);
        return pCacheMetadata;
    }

    Poco::SharedPtr<ByteArrayBuffer> setupByteArrayBuffer()
    {
        Poco::SharedPtr<ByteArrayBuffer> pData = new ByteArrayBuffer(Data1);
        return pData;
    }
};

namespace {
    bool isMetadataOuterMethodParameter(CacheMetadata::Ptr pCacheMetadata)
    {
        return (pCacheMetadata->getKey() == Key1);
    }

    bool isDataOuterMethodParameter(Poco::SharedPtr<ByteArrayBuffer> pData)
    {
        return (pData->toString() == Data1);
    }
}

// getMetadata
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.getMetadata(Key1, pCacheMetadata));
}

// getMetadata
// L1Cache のみ。
// L1Cache::getMetadata がtrue を返す。
//
// L1Cache::getMetadata が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call getMetadata
    // Then: return true and get metadata from cache.
    EXPECT_TRUE(cacheManager.getMetadata(Key1, pCacheMetadata));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
}

// getMetadata
// L1Cache のみ。
// L1Cache::getMetadata がfalse を返す。
//
// L1Cache::getMetadata が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.getMetadata(Key1, pCacheMetadata));
}

// getMetadata
// L2Cache のみ。
// L1Cache::getMetadata がtrue を返す。
//
// L2Cache::getMetadata が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::Return(true)));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata
    // Then: return true and get metadata from cache.
    EXPECT_TRUE(cacheManager.getMetadata(Key1, pCacheMetadata));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
}

// getMetadata
// L2Cache のみ。
// L1Cache::getMetadata がfalse を返す。
//
// L2Cache::getMetadata が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.getMetadata(Key1, pCacheMetadata));
}

// getMetadata
// L1Cache, L2Cache あり
// L1Cache::getMetadata がtrueを返す。
//
// L2Cache::getMetadata は呼び出されない。
// true が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsTrue_WhenL1CacheAndL2CacheAndL1CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::Return(true)));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getMetadata(Key1, testing::_)).Times(0);

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata
    // Then: return true and get metadata from cache.
    EXPECT_TRUE(cacheManager.getMetadata(Key1, pCacheMetadata));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
}

// getMetadata
// L1Cache, L2Cache あり
// L1Cache::getMetadata がfalseを返す。
// L2Cache::getMetadata がtrueを返す。
//
// L1Cache::getMetadata が呼び出される。
// L2Cache::getMetadata が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call getMetadata
    // Then: return true and get metadata from cache.
    EXPECT_TRUE(cacheManager.getMetadata(Key1, pCacheMetadata));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
}

// getMetadata
// L1Cache, L2Cache あり
// L1Cache::getMetadata がfalseを返す。
// L2Cache::getMetadata がfalseを返す。
//
// L1Cache::getMetadata が呼び出される。
// L2Cache::getMetadata が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getMetadata_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getMetadata(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call getMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.getMetadata(Key1, pCacheMetadata));
}

// getData
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return false
    EXPECT_FALSE(cacheManager.getData(Key1, pStream));
}

// getData
// L1Cache のみ。
// L1Cache::getData がtrue を返す。
//
// L1Cache::getData が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getData(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(Stream1), testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return true and get stream from cache
    EXPECT_TRUE(cacheManager.getData(Key1, pStream));
    EXPECT_EQ(Stream1, pStream);
}

// getData
// L1Cache のみ。
// L1Cache::getData がfalse を返す。
//
// L1Cache::getData が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getData(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return false
    EXPECT_FALSE(cacheManager.getData(Key1, pStream));
}

// getData
// L2Cache のみ。
// L1Cache::getData がtrue を返す。
//
// L2Cache::getData が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getData(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(Stream1), testing::Return(true)));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return true and get stream from cache
    EXPECT_TRUE(cacheManager.getData(Key1, pStream));
    EXPECT_EQ(Stream1, pStream);
}

// getData
// L2Cache のみ。
// L1Cache::getData がfalse を返す。
//
// L2Cache::getData が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getData(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return false
    EXPECT_FALSE(cacheManager.getData(Key1, pStream));
}

// getData
// L1Cache, L2Cache あり
// L1Cache::getData がtrueを返す。
//
// L2Cache::getData は呼び出されない。
// true が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsTrue_WhenL1CacheAndL2CacheAndL1CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getData(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(Stream1), testing::Return(true)));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getData(Key1, testing::_)).Times(0);

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return true and get stream from cache
    EXPECT_TRUE(cacheManager.getData(Key1, pStream));
    EXPECT_EQ(Stream1, pStream);
}

// getData
// L1Cache, L2Cache あり
// L1Cache::getData がfalseを返す。
// L2Cache::getData がtrueを返す。
//
// L1Cache::getData が呼び出される。
// L2Cache::getData が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getData(Key1, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getData(Key1, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(Stream1), testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return true and get stream from cache
    EXPECT_TRUE(cacheManager.getData(Key1, pStream));
    EXPECT_EQ(Stream1, pStream);
}

// getData
// L1Cache, L2Cache あり
// L1Cache::getData がfalseを返す。
// L2Cache::getData がfalseを返す。
//
// L1Cache::getData が呼び出される。
// L2Cache::getData が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, getData_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), getData(Key1, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), getData(Key1, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    std::istream* pStream = NULL;

    // When: call getData
    // Then: return false
    EXPECT_FALSE(cacheManager.getData(Key1, pStream));
}

// get
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return false
    EXPECT_FALSE(cacheManager.get(Key1, pCacheMetadata, pStream));
}

// get
// L1Cache のみ。
// L1Cache::get がtrue を返す。
//
// L1Cache::get が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::SetArgReferee<2>(Stream1),
            testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return true and get metadata and stream
    EXPECT_TRUE(cacheManager.get(Key1, pCacheMetadata, pStream));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
    EXPECT_EQ(Stream1, pStream);
}

// get
// L1Cache のみ。
// L1Cache::get がfalse を返す。
//
// L1Cache::get が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return false
    EXPECT_FALSE(cacheManager.get(Key1, pCacheMetadata, pStream));
}

// get
// L2Cache のみ。
// L1Cache::get がtrue を返す。
//
// L2Cache::get が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::SetArgReferee<2>(Stream1),
            testing::Return(true)));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return true
    EXPECT_TRUE(cacheManager.get(Key1, pCacheMetadata, pStream));
}

// get
// L2Cache のみ。
// L1Cache::get がfalse を返す。
//
// L2Cache::get が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return false
    EXPECT_FALSE(cacheManager.get(Key1, pCacheMetadata, pStream));
}

// get
// L1Cache, L2Cache あり
// L1Cache::get がtrueを返す。
//
// L2Cache::get は呼び出されない。
// true が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsTrue_WhenL1CacheAndL2CacheAndL1CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::SetArgReferee<2>(Stream1),
            testing::Return(true)));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), get(Key1, testing::_, testing::_)).Times(0);

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return true and get metadata and stream
    EXPECT_TRUE(cacheManager.get(Key1, pCacheMetadata, pStream));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
    EXPECT_EQ(Stream1, pStream);
}

// get
// L1Cache, L2Cache あり
// L1Cache::get がfalseを返す。
// L2Cache::get がtrueを返す。
//
// L1Cache::get が呼び出される。
// L2Cache::get が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(DoAll(testing::SetArgReferee<1>(setupCacheMetadata()), testing::SetArgReferee<2>(Stream1),
            testing::Return(true)));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return true and get metadata and stream
    EXPECT_TRUE(cacheManager.get(Key1, pCacheMetadata, pStream));
    EXPECT_EQ(Key1, pCacheMetadata->getKey());
    EXPECT_EQ(Stream1, pStream);
}

// get
// L1Cache, L2Cache あり
// L1Cache::get がfalseを返す。
// L2Cache::get がfalseを返す。
//
// L1Cache::get が呼び出される。
// L2Cache::get が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, get_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), get(Key1, testing::_, testing::_)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get
    // Then: return false
    EXPECT_FALSE(cacheManager.get(Key1, pCacheMetadata, pStream));
}

// putMetadata
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache のみ。
// L1Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return true
    EXPECT_TRUE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache のみ。
// L1Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L2Cache のみ。
// L1Cache::put が true を返す。
//
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return true
    EXPECT_TRUE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L2Cache のみ。
// L1Cache::put が false を返す。
//
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsTrue_WhenL1CacheReturnsTrueAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return true
    EXPECT_TRUE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return true
    EXPECT_TRUE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return false
    EXPECT_FALSE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// putMetadata
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putMetadata_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            putMetadata(Key1, testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), putMetadata(Key1,
            testing::Truly(isMetadataOuterMethodParameter))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata
    // Then: return true
    EXPECT_TRUE(cacheManager.putMetadata(Key1, pCacheMetadata));
}

// put with path
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache のみ。
// L1Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache のみ。
// L1Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L2Cache のみ。
// L1Cache::put が true を返す。
//
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L2Cache のみ。
// L1Cache::put が false を返す。
//
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsTrue_WhenL1CacheReturnsTrueAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with path
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithPath_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter), Path1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, Path1));
}

// put with buffer
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache のみ。
// L1Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache のみ。
// L1Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L2Cache のみ。
// L1Cache::put が true を返す。
//
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(NULL, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L2Cache のみ。
// L1Cache::put が false を返す。
//
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsTrue_WhenL1CacheReturnsTrueAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsTrue_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache, L2Cache あり
// L1Cache::put が false を返す。
// L2Cache::put が false を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return false
    EXPECT_FALSE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// put with buffer
// L1Cache, L2Cache あり
// L1Cache::put が true を返す。
// L2Cache::put が true を返す。
//
// L1Cache::put が呼び出される。
// L2Cache::put が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, putWithBuffer_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())),
            put(Key1, testing::Truly(isMetadataOuterMethodParameter),
            testing::Matcher<Poco::SharedPtr<ByteArrayBuffer> >(testing::Truly(isDataOuterMethodParameter)))).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);
    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put
    // Then: return true
    EXPECT_TRUE(cacheManager.put(Key1, pCacheMetadata, pData));
}

// remove
// L1Cache, L2Cache なし。
//
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsTrue_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L1Cache のみ。
// L1Cache::remove が true を返す。
//
// L1Cache::remove が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, NULL);

    // When: call remove
    // Then: return true
    EXPECT_TRUE(cacheManager.remove(Key1));
}

// remove
// L1Cache のみ。
// L1Cache::remove が false を返す。
//
// L1Cache::remove が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L2Cache のみ。
// L1Cache::remove が true を返す。
//
// L2Cache::remove が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(NULL, m_pMockL2Cache);

    // When: call remove
    // Then: return true
    EXPECT_TRUE(cacheManager.remove(Key1));
}

// remove
// L2Cache のみ。
// L1Cache::remove が false を返す。
//
// L2Cache::remove が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L1Cache, L2Cache あり
// L1Cache::remove が true を返す。
// L2Cache::remove が false を返す。
//
// L1Cache::remove が呼び出される。
// L2Cache::remove が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L1Cache, L2Cache あり
// L1Cache::remove が false を返す。
// L2Cache::remove が true を返す。
//
// L1Cache::remove が呼び出される。
// L2Cache::remove が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L1Cache, L2Cache あり
// L1Cache::remove が false を返す。
// L2Cache::remove が false を返す。
//
// L1Cache::remove が呼び出される。
// L2Cache::remove が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call remove
    // Then: return false
    EXPECT_FALSE(cacheManager.remove(Key1));
}

// remove
// L1Cache, L2Cache あり
// L1Cache::remove が true を返す。
// L2Cache::remove が true を返す。
//
// L1Cache::remove が呼び出される。
// L2Cache::remove が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, remove_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), remove(Key1)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call remove
    // Then: return true
    EXPECT_TRUE(cacheManager.remove(Key1));
}

// releaseData
// L1Cache のみ。。
//
// L1Cache::releaseData が呼び出される。

TEST_F(CacheManagerUnitTest, releaseData_CallsReleaseDataOfL1Cache_WhenL1CacheOnly)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), releaseData(Key1)).Times(1);

    CacheManager cacheManager(m_pMockL1Cache, NULL);

    // When: call releaseData
    // Then: be called L1Cache::releaseData
    cacheManager.releaseData(Key1);
}

// releaseData
// L2Cache のみ。。
//
// L2Cache::releaseData が呼び出される。

TEST_F(CacheManagerUnitTest, releaseData_CallsReleaseDataOfL2Cache_WhenL2CacheOnly)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), releaseData(Key1)).Times(1);

    CacheManager cacheManager(NULL, m_pMockL2Cache);

    // When: call releaseData
    // Then: be called L2Cache::releaseData
    cacheManager.releaseData(Key1);
}

// releaseData
// L1Cache, L2Cache あり
//
// L1Cache::releaseData が呼び出される。
// L2Cache::releaseData が呼び出される。

TEST_F(CacheManagerUnitTest, releaseData_CallsReleaseDataOfL1CacheAndL2Cache_WhenExistsL1CacheAndL2Cache)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), releaseData(Key1)).Times(1);
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), releaseData(Key1)).Times(1);

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call releaseData
    // Then: be called L1Cache::releaseData and L2Cache::reelaseData
    cacheManager.releaseData(Key1);
}

// purge
// L1Cache, L2Cache なし。
//
// true が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenNoL1CacheAndNoL2Cache)
{
    // Given: no L1Cache, no L2Cache
    CacheManager cacheManager(NULL, NULL);

    // When: call purge
    // Then: return true
    EXPECT_TRUE(cacheManager.purge(true));
}

// purge
// L1Cache のみ。
// L1Cache::purge が true を返す。
//
// L1Cache::purge が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsTrue_WhenL1CacheOnlyAndL1CacheReturnsTrue)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, NULL);

    // When: call purge
    // Then: return true
    EXPECT_TRUE(cacheManager.purge(true));
}

// purge
// L1Cache のみ。
// L1Cache::purge が false を返す。
//
// L1Cache::purge が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL1CacheOnlyAndL1CacheReturnsFalse)
{
    // Given: L1Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, NULL);

    // When: call purge
    // Then: return false
    EXPECT_FALSE(cacheManager.purge(true));
}

// purge
// L2Cache のみ。
// L1Cache::purge が true を返す。
//
// L2Cache::purge が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsTrue_WhenL2CacheOnlyAndL2CacheReturnsTrue)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(false)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(NULL, m_pMockL2Cache);

    // When: call purge
    // Then: return true
    EXPECT_TRUE(cacheManager.purge(false));
}

// purge
// L2Cache のみ。
// L1Cache::purge が false を返す。
//
// L2Cache::purge が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL2CacheOnlyAndL2CacheReturnsFalse)
{
    // Given: L2Cache only
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(false)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL2Cache, NULL);

    // When: call purge
    // Then: return false
    EXPECT_FALSE(cacheManager.purge(false));
}

// purge
// L1Cache, L2Cache あり
// L1Cache::purge が true を返す。
// L2Cache::purge が false を返す。
//
// L1Cache::purge が呼び出される。
// L2Cache::purge が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(true)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call purge
    // Then: return false
    EXPECT_FALSE(cacheManager.purge(true));
}

// purge
// L1Cache, L2Cache あり
// L1Cache::purge が false を返す。
// L2Cache::purge が true を返す。
//
// L1Cache::purge が呼び出される。
// L2Cache::purge が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(true)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call purge
    // Then: return false
    EXPECT_FALSE(cacheManager.purge(true));
}

// purge
// L1Cache, L2Cache あり
// L1Cache::purge が false を返す。
// L2Cache::purge が false を返す。
//
// L1Cache::purge が呼び出される。
// L2Cache::purge が呼び出される。
// false が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL1CacheReturnsFalseAndL2CacheReturnsFalse)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(false));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(true)).
            WillOnce(testing::Return(false));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call purge
    // Then: return false
    EXPECT_FALSE(cacheManager.purge(true));
}

// purge
// L1Cache, L2Cache あり
// L1Cache::purge が true を返す。
// L2Cache::purge が true を返す。
//
// L1Cache::purge が呼び出される。
// L2Cache::purge が呼び出される。
// true が返る。

TEST_F(CacheManagerUnitTest, purge_ReturnsFalse_WhenL1CacheReturnsTrueAndL2CacheReturnsTrue)
{
    // Given: L1Cache and L2Cache
    testing::InSequence seq;
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL1Cache.get())), purge(true)).
            WillOnce(testing::Return(true));
    EXPECT_CALL(*(static_cast<MockCache*> (m_pMockL2Cache.get())), purge(true)).
            WillOnce(testing::Return(true));

    CacheManager cacheManager(m_pMockL1Cache, m_pMockL2Cache);

    // When: call purge
    // Then: return true
    EXPECT_TRUE(cacheManager.purge(true));
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
