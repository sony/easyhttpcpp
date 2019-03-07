/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/Thread.h"
#include "Poco/ThreadPool.h"

#include "easyhttpcpp/common/CacheManager.h"
#include "TestLogger.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace common {
namespace test {

static const std::string Tag = "CacheManagerUnitTest";
static const std::string Key1 = "key1";
static const std::string Path1 = "tmp/file.dat";
static const std::string Data1 = "test_data";
static const int TestFailureTimeout = 10 * 1000; // milliseconds

class CacheManagerMultiThreadUnitTest : public testing::Test {
};

namespace {

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

class MultiThreadTestL2Cache : public Cache {
public:
    MultiThreadTestL2Cache(Poco::Thread::TID mainThreadId) : m_mainThreadId(mainThreadId), m_executeL2(false)
    {
    }
    virtual bool getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool getData(const std::string& key, std::istream*& pStream)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<ByteArrayBuffer> pData)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual bool remove(const std::string& key)
    {
        setExecuteIfFirstCache();
        return true;
    }
    virtual void releaseData(const std::string& key)
    {
        setExecuteIfFirstCache();
    }
    virtual bool purge(bool mayDeleteIfBusy)
    {
        setExecuteIfFirstCache();
        return true;
    }

    bool isExecuted()
    {
        return m_executeL2;
    }

private:
    void setExecuteIfFirstCache()
    {
        if (Poco::Thread::currentTid() != m_mainThreadId) {
            EASYHTTPCPP_TESTLOG_I(Tag, "set L2 of 1st. cache execute");
            m_executeL2 = true;
        }
    }
    Poco::Thread::TID m_mainThreadId;
    bool m_executeL2;
};

class FirstExecuteCache : public Poco::Runnable {
public:

    FirstExecuteCache(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            m_cacheManager(cacheManager), m_mainThreadId(mainThreadId)
    {
    }

    virtual void run()
    {
        execute();
    }
protected:
    virtual void execute() = 0;

    CacheManager& m_cacheManager;
    Poco::Thread::TID m_mainThreadId;
};

class MultiThreadTestL1Cache : public Cache {
public:
    MultiThreadTestL1Cache(Poco::Thread::TID mainThreadId, MultiThreadTestL2Cache* pL2Cache) :
            m_mainThreadId(mainThreadId), m_pL2Cache(pL2Cache), m_executeAfterFirstCache(false)
    {
    }
    virtual bool getMetadata(const std::string& key, CacheMetadata::Ptr& pCacheMetadata)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return false;
    }
    virtual bool getData(const std::string& key, std::istream*& pStream)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return false;
    }
    virtual bool get(const std::string& key, CacheMetadata::Ptr& pCacheMetadata, std::istream*& pStream)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return false;
    }
    virtual bool putMetadata(const std::string& key, CacheMetadata::Ptr pCacheMetadata)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return true;
    }
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata, const std::string& path)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return true;
    }
    virtual bool put(const std::string& key, CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<ByteArrayBuffer> pData)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return true;
    }
    virtual bool remove(const std::string& key)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return true;
    }
    virtual void releaseData(const std::string& key)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
    }
    virtual bool purge(bool mayDeleteIfBusy)
    {
        startIfFirstCache();
        checkExecuteOrderIfSecondCache();
        return true;
    }

    bool wait()
    {
        if (!m_firstCacheStartEvent.tryWait(TestFailureTimeout)) {
            return false;
        }
        return true;
    }
    bool isExecuteAfterFirstCache()
    {
        return m_executeAfterFirstCache;
    }
private:
    void startIfFirstCache()
    {
        if (Poco::Thread::currentTid() != m_mainThreadId) {
            m_firstCacheStartEvent.set();
        }
    }
    void checkExecuteOrderIfSecondCache()
    {
        if (Poco::Thread::currentTid() == m_mainThreadId) {
            if (m_pL2Cache->isExecuted()) {
                EASYHTTPCPP_TESTLOG_I(Tag, "set L1 of 2nd. cache execute");
                m_executeAfterFirstCache = true;
            }
        }
    }
    Poco::Thread::TID m_mainThreadId;
    Poco::Event m_firstCacheStartEvent;
    MultiThreadTestL2Cache* m_pL2Cache;
    bool m_executeAfterFirstCache;
};

class FirstExecuteCacheGetMetadata : public FirstExecuteCache {
public:
    FirstExecuteCacheGetMetadata(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        CacheMetadata::Ptr pCacheMetadata;
        m_cacheManager.getMetadata(Key1, pCacheMetadata);
    }
};

class FirstExecuteCacheGetData : public FirstExecuteCache {
public:
    FirstExecuteCacheGetData(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        std::istream* pStream = NULL;
        m_cacheManager.getData(Key1, pStream);
    }
};

class FirstExecuteCacheGet : public FirstExecuteCache {
public:
    FirstExecuteCacheGet(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        CacheMetadata::Ptr pCacheMetadata;
        std::istream* pStream = NULL;
        m_cacheManager.get(Key1, pCacheMetadata, pStream);
    }
};

class FirstExecuteCachePutMetadata : public FirstExecuteCache {
public:
    FirstExecuteCachePutMetadata(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
        m_cacheManager.putMetadata(Key1, pCacheMetadata);
    }
};

class FirstExecuteCachePutWithPath : public FirstExecuteCache {
public:
    FirstExecuteCachePutWithPath(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
        m_cacheManager.put(Key1, pCacheMetadata, Path1);
    }
};

class FirstExecuteCachePutWithBuffer : public FirstExecuteCache {
public:
    FirstExecuteCachePutWithBuffer(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
        Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();
        m_cacheManager.put(Key1, pCacheMetadata, pData);
    }
};

class FirstExecuteCacheRemove : public FirstExecuteCache {
public:
    FirstExecuteCacheRemove(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        m_cacheManager.remove(Key1);
    }
};

class FirstExecuteCacheReleaseData : public FirstExecuteCache {
public:
    FirstExecuteCacheReleaseData(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        m_cacheManager.releaseData(Key1);
    }
};

class FirstExecuteCachePurge : public FirstExecuteCache {
public:
    FirstExecuteCachePurge(CacheManager& cacheManager, Poco::Thread::TID mainThreadId) :
            FirstExecuteCache(cacheManager, mainThreadId)
    {
    }
    virtual void execute()
    {
        m_cacheManager.purge(true);
    }
};

} /* namespace */

TEST(CacheManagerMultiThreadUnitTest, getMetadata_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call getMetadata in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCacheGetMetadata firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    CacheMetadata::Ptr pCacheMetadata;

    // When: call getMetadata in thread2
    cacheManager.getMetadata(Key1, pCacheMetadata);
    threadPool.joinAll();

    // Then: wait to execute thread2 getMetadata
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, getData_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call getData in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCacheGetData firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    std::istream* pStream = NULL;

    // When: call getData in thread2
    cacheManager.getData(Key1, pStream);
    threadPool.joinAll();

    // Then: wait to execute thread2 getData
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, get_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call getData in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCacheGet firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    CacheMetadata::Ptr pCacheMetadata;
    std::istream* pStream = NULL;

    // When: call get in thread2
    cacheManager.get(Key1, pCacheMetadata, pStream);
    threadPool.joinAll();

    // Then: wait to execute thread2 get
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, putMetadata_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call putMetadata in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCachePutMetadata firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call putMetadata in thread2
    cacheManager.putMetadata(Key1, pCacheMetadata);
    threadPool.joinAll();

    // Then: wait to execute thread2 putMetadata
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, putWithPath_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call put with path in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCachePutWithPath firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();

    // When: call put with path in thread2
    cacheManager.put(Key1, pCacheMetadata, Path1);
    threadPool.joinAll();

    // Then: wait to execute thread2 put with path
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, putWithBuffer_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call put with buffer in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCachePutWithBuffer firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    CacheMetadata::Ptr pCacheMetadata = setupCacheMetadata();
    Poco::SharedPtr<ByteArrayBuffer> pData = setupByteArrayBuffer();

    // When: call put with buffer in thread2
    cacheManager.put(Key1, pCacheMetadata, pData);
    threadPool.joinAll();

    // Then: wait to execute thread2 put with buffer
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, remove_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call remove in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCacheRemove firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    // When: call remove in thread2
    cacheManager.remove(Key1);
    threadPool.joinAll();

    // Then: wait to execute thread2 remove
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, releaseData_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call releaseData in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCacheReleaseData firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    // When: call releaseData in thread2
    cacheManager.releaseData(Key1);
    threadPool.joinAll();

    // Then: wait to execute thread2 releaseData
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

TEST(CacheManagerMultiThreadUnitTest, purge_ExcutesInSequential_WhenCallFromMultiThread)
{
    // Given: call purge in thread1 and wait to start thread
    Poco::Thread::TID tid = Poco::Thread::currentTid();
    MultiThreadTestL2Cache* pL2Cache = new MultiThreadTestL2Cache(tid);
    MultiThreadTestL1Cache* pL1Cache = new MultiThreadTestL1Cache(tid, pL2Cache);
    CacheManager cacheManager(pL1Cache, pL2Cache);

    Poco::ThreadPool threadPool;
    FirstExecuteCachePurge firstExecuteCache(cacheManager, tid);
    threadPool.start(firstExecuteCache);

    ASSERT_TRUE(pL1Cache->wait());

    // When: call purge in thread2
    cacheManager.purge(true);
    threadPool.joinAll();

    // Then: wait to execute thread2 purge
    EXPECT_TRUE(pL1Cache->isExecuteAfterFirstCache());
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
