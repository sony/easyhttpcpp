/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"
#include "Poco/ThreadPool.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/Request.h"

#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "EasyHttpContext.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const char* const Tag = "ConnectionPoolInternalMultiThreadIntegrationTest";
static const unsigned int PreparedConnectionCount = 50; // getConnection に時間をかけるため、用意する Connection 数
static const int TestFailureTimeout = 10 * 1000;        // milliseconds
static const unsigned int MultiThreadCount = 20;        // 同時に実行する Thread 数
static const unsigned long DefaultKeepAliveTimeoutSec = 60;

namespace {

class ConnectionPoolExecutionRunner : public Poco::Runnable, public Poco::RefCountedObject {
public:
    ConnectionPoolExecutionRunner(ConnectionPoolInternal::Ptr pConnectionPoolInternal, unsigned int id) :
            m_id(id), m_pConnectionPoolInternal(pConnectionPoolInternal), m_result(false)
    {
    }

    virtual void run()
    {
        m_beforeExecuteEvent.set();        
        if (!m_startToExecuteEvent.tryWait(TestFailureTimeout)) {
            EASYHTTPCPP_LOG_E(Tag, "start to execute event is time out. id=%u", m_id);
            m_result = false;
            return;
        }
        m_result = execute();
    }

    bool waitBeforeExecute()
    {
        return m_beforeExecuteEvent.tryWait(TestFailureTimeout);
    }

    void setStartToExecute()
    {
        m_startToExecuteEvent.set();
    }

    bool getResult()
    {
        return m_result;
    }

protected:
    virtual bool execute() = 0;

    unsigned int m_id;
    ConnectionPoolInternal::Ptr m_pConnectionPoolInternal;

private:
    bool m_result;
    Poco::Event m_beforeExecuteEvent;
    Poco::Event m_startToExecuteEvent;
};

class GetConnectionExecutionRunner : public ConnectionPoolExecutionRunner {
public:
    GetConnectionExecutionRunner(ConnectionPoolInternal::Ptr pConnectionPoolInternal,
            EasyHttpContext::Ptr pEasyHttpContext, unsigned int id, Request::Ptr pRequest) :
            ConnectionPoolExecutionRunner(pConnectionPoolInternal, id),
            m_pEasyHttpContext(pEasyHttpContext), m_pRequest(pRequest), m_connectionReused(false)
    {
    }

    ConnectionInternal::Ptr getConnectionInternal()
    {
        return m_pConnectionInternal;
    }
    bool isConnectionReused()
    {
        return m_connectionReused;
    }

protected:
    virtual bool execute()
    {
        m_pConnectionInternal = m_pConnectionPoolInternal->getConnection(m_pRequest, m_pEasyHttpContext,
                m_connectionReused);
        return true;
    }

private:
    EasyHttpContext::Ptr m_pEasyHttpContext;
    Request::Ptr m_pRequest;
    ConnectionInternal::Ptr m_pConnectionInternal;
    bool m_connectionReused;
};

class RemoveConnectionExecutionRunner : public ConnectionPoolExecutionRunner {
public:
    RemoveConnectionExecutionRunner(ConnectionPoolInternal::Ptr pConnectionPoolInternal, unsigned int id,
            ConnectionInternal::Ptr pConnectionInternal) :
            ConnectionPoolExecutionRunner(pConnectionPoolInternal, id), m_pConnectionInternal(pConnectionInternal)
    {
    }

protected:
    virtual bool execute()
    {
        EASYHTTPCPP_LOG_D(Tag, "RemoveConnectionExecutionRunner begin removeConnection");
        bool ret = m_pConnectionPoolInternal->removeConnection(m_pConnectionInternal);
        EASYHTTPCPP_LOG_D(Tag, "RemoveConnectionExecutionRunner end removeConnection");
        return ret;
    }
private:
    ConnectionInternal::Ptr m_pConnectionInternal;
};

class ReleaseConnectionExecutionRunner : public ConnectionPoolExecutionRunner {
public:
    ReleaseConnectionExecutionRunner(ConnectionPoolInternal::Ptr pConnectionPoolInternal, unsigned int id,
            ConnectionInternal::Ptr pConnectionInternal) :
            ConnectionPoolExecutionRunner(pConnectionPoolInternal, id), m_pConnectionInternal(pConnectionInternal)
    {
    }

protected:
    virtual bool execute()
    {
        EASYHTTPCPP_LOG_D(Tag, "ReleaseConnectionExecutionRunner begin releaseConnection");
        bool ret = m_pConnectionPoolInternal->releaseConnection(m_pConnectionInternal);
        EASYHTTPCPP_LOG_D(Tag, "ReleaseConnectionExecutionRunner end releaseConnection");
        return ret;
    }

private:
    ConnectionInternal::Ptr m_pConnectionInternal;
};

}

class ConnectionPoolInternalMultiThreadIntegrationTest : public HttpIntegrationTestCase {
protected:
    void SetUp()
    {
        Poco::Path cachePath(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(cachePath);

        Poco::Path certRootDir(HttpTestUtil::getDefaultCertRootDir());
        FileUtil::removeDirsIfPresent(certRootDir);
    }

    Poco::AutoPtr<ConnectionPoolExecutionRunner> m_pRunners[MultiThreadCount];
};

// Multi Thread からの同時 getConnection で Connection が再利用される。 (http)
//
// 1. getConnection の検索に時間がかかるよう多く Connection を用意する。
// 2. Multi Thread から、再利用できる host で同時に getConnection を呼び出す。
// 確認項目
// それぞれ Connection が再利用される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        getConnection_GetsConnection_WhenCalledFromMultipleThreadsAtTheSameTimeWithSameHost)
{
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(PreparedConnectionCount, DefaultKeepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: 再利用するための idle の Connection を用意する。
    // getConnection の検索に時間がかかるよう多くの Connection を用意する。
    for (unsigned int i = 0; i < PreparedConnectionCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        pConnectionPoolInternal->releaseConnection(pConnectionInternal);
    }

    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // 複数の Thread で、同時に getConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::getConnection を呼び出す直前で待機。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        m_pRunners[i] = new GetConnectionExecutionRunner(pConnectionPoolInternal, pEasyHttpContext, i, pRequest);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }

    // When: 全 thread から ConnectionPoolInternal::getConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全 thread で 再利用した Connection が取得される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
        GetConnectionExecutionRunner* pGetConnectionExecutionRunner =
            static_cast<GetConnectionExecutionRunner*>(m_pRunners[i].get());
        EXPECT_FALSE(pGetConnectionExecutionRunner->getConnectionInternal().isNull()) << StringUtil::format("id=%u", i);
        EXPECT_TRUE(pGetConnectionExecutionRunner->isConnectionReused()) << StringUtil::format("id=%u", i);
    }
    // ConnectionPoolInternal の Connection の数に変化なし。
    EXPECT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    // 全て、inuse (idle ではない) となっている。
    EXPECT_EQ(PreparedConnectionCount - MultiThreadCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// Multi Thread からの同時 getConnection で Connection が再利用される。 (https)
//
// 1. getConnection の検索に時間がかかるよう多く Connection を用意する。
// 2. Multi Thread から、再利用できる host で同時に getConnection を呼び出す。
// 確認項目
// それぞれ Connection が再利用される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        getConnection_GetsConnection_WhenSchemeIsHttpsAndCalledFromMultipleThreadsAtTheSameTimeWithSameHost)
{
    // load cert data
    HttpTestUtil::loadDefaultCertData();

    // Given: 再利用するための idle の Connection を用意する。
    // getConnection の検索に時間がかかるよう多くの Connection を用意する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(PreparedConnectionCount, DefaultKeepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setRootCaDirectory(HttpTestUtil::getDefaultRootCaDirectory());

    for (unsigned int i = 0; i < PreparedConnectionCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        pConnectionPoolInternal->releaseConnection(pConnectionInternal);
    }

    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // 複数の Thread で、同時に getConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::getConnection を呼び出す直前で待機。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        m_pRunners[i] = new GetConnectionExecutionRunner(pConnectionPoolInternal, pEasyHttpContext, i, pRequest);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }

    // When: 全 thread から ConnectionPoolInternal::getConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全 thread で 再利用した Connection が取得される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
        GetConnectionExecutionRunner* pGetConnectionExecutionRunner =
            static_cast<GetConnectionExecutionRunner*>(m_pRunners[i].get());
        EXPECT_FALSE(pGetConnectionExecutionRunner->getConnectionInternal().isNull()) << StringUtil::format("id=%u", i);
        EXPECT_TRUE(pGetConnectionExecutionRunner->isConnectionReused()) << StringUtil::format("id=%u", i);
    }
    // ConnectionPool の Connection の数に変化なし。
    EXPECT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    // 全て、inuse (idle ではない) となっている。
    EXPECT_EQ(PreparedConnectionCount - MultiThreadCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// Multi Thread からの同時 getConnection で Connection が生成される。 (http)
//
// 1. getConnection の検索に時間がかかるよう多く Connection を用意する。
// 2. Multi Thread から、別の host で同時に getConnection を呼び出す。
// 確認項目
// それぞれ Connection が生成される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        getConnection_CreatesConnection_WhenCalledFromMultipleThreadsAtTheSameTimeWithDifferentHost)
{
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(PreparedConnectionCount, DefaultKeepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: 再利用されない Connection を用意する。
    // getConnection の検索に時間がかかるよう多くの Connection を用意する。
    for (unsigned int i = 0; i < PreparedConnectionCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        pConnectionPoolInternal->releaseConnection(pConnectionInternal);
    }

    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // 複数の Thread で、同時に getConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::getConnection を呼び出す直前で待機。
    // 再利用されないように、host 名を変える。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("differenthost%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        m_pRunners[i] = new GetConnectionExecutionRunner(pConnectionPoolInternal, pEasyHttpContext, i, pRequest);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }

    // When: 全 thread から ConnectionPoolInternal::getConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全 thread で 新規に Connection が生成される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
        GetConnectionExecutionRunner* pGetConnectionExecutionRunner =
            static_cast<GetConnectionExecutionRunner*>(m_pRunners[i].get());
        EXPECT_FALSE(pGetConnectionExecutionRunner->getConnectionInternal().isNull()) << StringUtil::format("id=%u", i);
        EXPECT_FALSE(pGetConnectionExecutionRunner->isConnectionReused()) << StringUtil::format("id=%u", i);
    }
    EXPECT_EQ(PreparedConnectionCount + MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());
    // idle の Connection 数は変化なし(再利用されないため)
    EXPECT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// Multi Thread からの同時 getConnection で Connection が生成される。 (https)
//
// 1. getConnection の検索に時間がかかるよう多く Connection を用意する。
// 2. Multi Thread から、別の host で同時に getConnection を呼び出す。
// 確認項目
// それぞれ Connection が生成される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        getConnection_CreatesConnection_WhenSchemeIsHttpsAndCalledFromMultipleThreadsAtTheSameTimeWithDifferentHost)
{
    // load cert data
    HttpTestUtil::loadDefaultCertData();

    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(PreparedConnectionCount, DefaultKeepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    std::string certRootDir = HttpTestUtil::getDefaultCertRootDir();
    pEasyHttpContext->setRootCaDirectory(HttpTestUtil::getDefaultRootCaDirectory());

    // Given: 再利用されない Connection を用意する。
    // getConnection の検索に時間がかかるよう多くの Connection を用意する。
    for (unsigned int i = 0; i < PreparedConnectionCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        pConnectionPoolInternal->releaseConnection(pConnectionInternal);
    }

    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // 複数の Thread で、同時に getConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::getConnection を呼び出す直前で待機。
    // 再利用されないように、host 名を変える。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Https, StringUtil::format("differenthost%03u", i),
                HttpTestConstants::DefaultHttpsPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        m_pRunners[i] = new GetConnectionExecutionRunner(pConnectionPoolInternal, pEasyHttpContext, i, pRequest);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }

    // When: 全 thread から ConnectionPoolInternal::getConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全 thread で 新規に Connection が生成される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
        GetConnectionExecutionRunner* pGetConnectionExecutionRunner =
            static_cast<GetConnectionExecutionRunner*>(m_pRunners[i].get());
        EXPECT_FALSE(pGetConnectionExecutionRunner->getConnectionInternal().isNull()) << StringUtil::format("id=%u", i);
        EXPECT_FALSE(pGetConnectionExecutionRunner->isConnectionReused()) << StringUtil::format("id=%u", i);
    }
    EXPECT_EQ(PreparedConnectionCount + MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());
    // idle の Connection 数は変化なし(再利用されないため)
    EXPECT_EQ(PreparedConnectionCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// Multi Thread から、同時に removeConnection を呼び出す。
//
// 確認項目
// ConnectionPool から Connection が remove される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        removeConnection_RemovesConnection_WhenCalledFromMultipleThreads)
{
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: 複数の Connection を用意する。

    // 複数の Thread で、同時に removeConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::removeConnection を呼び出す直前で待機。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        m_pRunners[i] = new RemoveConnectionExecutionRunner(pConnectionPoolInternal, i, pConnectionInternal);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }
    ASSERT_EQ(MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());

    // When: 全 thread から ConnectionPoolInternal::removeConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: それぞれの Connection が ConnetionPool から remove される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
    }
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// Multi Thread から、同時に releaseConnection を呼び出す。(keep-Alive timeout 開始)
//
// 1. max keep-Alive idle 数をThreadの数以上にする。
// 2. 複数のThread から、別々の Connection で、releaseConnectionを同時に呼び出す。
// 確認項目
// それぞれのConnection がidle となる。
// keep-Alive の timeout 時間経過後に、ConnectionPool から全てのidle の Connection がremove される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        releaseConnection_ChangesStatusToIdleAndStartsKeepAliveTimeout_WhenCalledFromMultipleThreadsAndConnectionIsLessThanMaxKeepAliveIdleCount)
{
    // Given: ConnectionPool の max keep-Alive idle 数を thread 数以上にする。
    // 複数の Connection を用意する。
    unsigned long keepAliveTimeoutSec = 3;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool(MultiThreadCount,
            keepAliveTimeoutSec).unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // 複数の Thread で、同時に releaseConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::releaseConnection を呼び出す直前で待機。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        m_pRunners[i] = new ReleaseConnectionExecutionRunner(pConnectionPoolInternal, i, pConnectionInternal);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }
    ASSERT_EQ(MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: 全 thread から ConnectionPoolInternal::releaseConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全ての Connection が idle で、keep-Alive timeout 待ちとなる。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
    }
    ASSERT_EQ(MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(MultiThreadCount, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // keep-Alive timeout 時間経過後に、ConnectionPool から全ての Connection が削除される。
    Poco::Thread::sleep(keepAliveTimeoutSec * 1000 + 500);  // margin 500ms
    ASSERT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// Multi Thread から、同時に releaseConnection を呼び出す。
// max keep-Alive idle 数の上限による、ConnectionPool からの削除も同時に行う。
//
// 1. max Kep-Alive idle 数を 0 する。
// 2. 複数のThread から、別々の Connection で、releaseConnectionを同時に呼び出す。
// 確認項目
// ConnectionPool から全てのConnection が削除される。
TEST_F(ConnectionPoolInternalMultiThreadIntegrationTest,
        releaseConnection_RemovesIdleConnection_WhenCalledFromMultipleThreadsAndMaxKeepAliveIdleCountIsZero)
{
    // Given: ConnectionPool の max keep-Alive idle 数を 0 にする。
    // 複数の Connection を用意する。
    unsigned long keepAliveTimeoutSec = 3;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool(0, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // 複数の Thread で、同時に releaseConnection を呼び出すため、
    // 各 Thread の ConnectionPoolInternal::releaseConnection を呼び出す直前で待機。
    Poco::ThreadPool threadPool(1, MultiThreadCount);
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, StringUtil::format("host%03u", i),
                HttpTestConstants::DefaultPort, HttpTestConstants::DefaultPath);
        Request::Builder requestBuilder;
        Request::Ptr pRequest = requestBuilder.setUrl(url).build();
        ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest,
                pEasyHttpContext);
        m_pRunners[i] = new ReleaseConnectionExecutionRunner(pConnectionPoolInternal, i, pConnectionInternal);
        threadPool.start(*m_pRunners[i]);
        ASSERT_TRUE(m_pRunners[i]->waitBeforeExecute()) << StringUtil::format("id=%u", i);
    }
    ASSERT_EQ(MultiThreadCount, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: 全 thread から ConnectionPoolInternal::releaseConnection を呼び出す。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        m_pRunners[i]->setStartToExecute();
    }

    threadPool.stopAll();

    // Then: 全ての Connection が ConnectionPool から削除される。
    for (unsigned int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pRunners[i]->getResult());
    }
    ASSERT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

} /* namespace test */
} /* namespace easyhttpcpp */
