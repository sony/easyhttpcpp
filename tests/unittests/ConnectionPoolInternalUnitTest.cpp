/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Timestamp.h"

#include "easyhttpcpp/HttpException.h"
#include "MockRequest.h"
#include "EasyHttpCppAssertions.h"

#include "ConnectionPoolInternal.h"
#include "ConnectionInternal.h"
#include "KeepAliveTimeoutTask.h"

using easyhttpcpp::testutil::MockRequest;

namespace easyhttpcpp {
namespace test {

namespace {

static const std::string UnDecodableUrl = "http://host01/undecodable%";

} /* namespace */

// getTotalConnectionCount
TEST(ConnectionPoolInternalUnitTest,
        getTotalConnectionCount_ReturnsConnectionCountInConnectionPool_WhenConnectionExistsInConnectionPool)
{
    // Given: create default ConnectionPool
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    // When: increase, decrease Connection in Connection.
    // Then: Total Connection Count increase or decrease.

    // Connection not exist.
    EXPECT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // add Connection.
    // one Inuse Connection.
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // change Inuse to Tdle.
    // one Idle Connection.
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // add Inuse Connection.
    // one Inuse Connection and one Idle Connection.
    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());
    EXPECT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());

    // remove one Connection.
    pConnectionPoolInternal->removeConnection(pConnectionInternal1);
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // remove one Connection.
    pConnectionPoolInternal->removeConnection(pConnectionInternal2);
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// getKeepAliveIdleConnectionCount
TEST(ConnectionPoolInternalUnitTest,
        getKeepAliveIdleConenctionCount_ReturnsKeepAliveIdleConnectionCount_WhenKeepAliveIdleConnectionExistsInConnectionPool)
{
    // Given: create default ConnectionPool
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    // When: increase, decrease KeepAlive Idle Connection in Connection.
    // Then: KeepAlive Idle Connection Count increase or decrease.

    // Connection not exist.
    EXPECT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // add Connection
    // no Idle Connection.
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // change Inuse to Idle.
    // one Idle Connection.
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // add Connection
    // one Idle Connection and one Inuse Connection.
    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());
    EXPECT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // change Inuse to Idle
    // two Idle Connection.
    pConnectionPoolInternal->releaseConnection(pConnectionInternal2);
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal2->getStatus());
    EXPECT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(2, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// ConnectionPool が空の時の getConnection 呼び出し。
// 新しい Connection が作成され、ConnectionPool に追加される。
// connectionReused = false
TEST(ConnectionPoolInternalUnitTest, getConnection_CreatesConnection_WhenConnectinPoolIsEmpty)
{
    // Given: 空の ConnectionPool
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // When: call getConnection
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    bool connectionReused = true;
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->getConnection(pRequest1, pEasyHttpContext,
            connectionReused);

    // Then: 新しい Connection が生成される。
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());
    EXPECT_FALSE(connectionReused);
}

// ConnectionPool に別の host の idle の Connection のみがある場合の、getConnection 呼び出し。
// 新しい Connection が作成され、ConnectionPool に追加される。
// connectionReused = false
TEST(ConnectionPoolInternalUnitTest,
        getConnection_CreatesConnection_WhenOnlyDifferentHostConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に host の違う Connection を追加する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);

    // When: call getConnection
    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    bool connectionReused = true;
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->getConnection(pRequest2, pEasyHttpContext,
            connectionReused);

    // Then: 新しい Connection が生成される。
    EXPECT_NE(pConnectionInternal1, pConnectionInternal2);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());
    EXPECT_FALSE(connectionReused);
}

// ConnectionPool に同じ host の Inuse の Connection がある場合の、getConnection 呼び出し。
// 新しい Connection が作成され、ConnectionPool に追加される。
// connectionReused = false
TEST(ConnectionPoolInternalUnitTest, getConnection_CreatesConnection_WhenSameHostInuseConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に Inuse の Connection を追加する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());

    // When: call getConnection
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    bool connectionReused = true;
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->getConnection(pRequest2, pEasyHttpContext,
            connectionReused);

    // Then: 新しい Connection が生成される。
    EXPECT_NE(pConnectionInternal1, pConnectionInternal2);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());
    EXPECT_FALSE(connectionReused);
    ASSERT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に同じ host の idle の Connection がある場合の、getConnection 呼び出し。
// Connection が再利用される。
// connectionReused = true
TEST(ConnectionPoolInternalUnitTest, getConnection_ReusesConnection_WhenSameHostIdleConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に Idle の Connection を追加する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // When: call getConnection
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    bool connectionReused = false;
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->getConnection(pRequest2, pEasyHttpContext,
            connectionReused);

    // Then: Idle の Connection が再利用される。
    EXPECT_EQ(pConnectionInternal1, pConnectionInternal2);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());
    EXPECT_TRUE(connectionReused);
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に同じhost のInuse の Connection と同じ host のIdle の  Connection がある場合の、getConnection 呼び出し。
// Idle の Conenction が再利用される。
// connectionReused = true
TEST(ConnectionPoolInternalUnitTest,
        getConnection_ReusesSameHostIdleConnection_WhenSameHostIdleConnectionAndSameHostInuseConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に、Idle の Connection と Inuse の Connection を追加する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Idle の Connection
    std::string url = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // Inuse の Connection
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);
    ASSERT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal2->getStatus());

    // When: call getConnection
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url).build();
    bool connectionReused = false;
    ConnectionInternal::Ptr pConnectionInternal3 = pConnectionPoolInternal->getConnection(pRequest3, pEasyHttpContext,
            connectionReused);

    // Then: Idle の Connection が再利用される。
    EXPECT_EQ(pConnectionInternal1, pConnectionInternal3);
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal3->getStatus());
    EXPECT_TRUE(connectionReused);
    ASSERT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// ConnectionPool が空の時の createConnection
// Connection が作成され ConnectionPool に追加される。
// 指定した proxy, timeout が HTTPClientSession に設定される。
// HTTPClientSession の keepAlive == true
// KeepAliveTimeout が、keepAliveTimeout + 1日。
TEST(ConnectionPoolInternalUnitTest,
        createConnection_CreatesConnectionWithSpecifiedParameterAndAddToConnectionPool_WhenConnectinPoolIsEmpty)
{
    // Given: ConnectionPool is empty.
    unsigned int keepAliveIdleCountMax = 5;
    unsigned long keepAliveTimeoutSec = 20;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    Proxy::Ptr pProxy = new Proxy("proxyHost", 1234);
    pEasyHttpContext->setProxy(pProxy);
    unsigned int timeoutSec = 10;
    pEasyHttpContext->setTimeoutSec(timeoutSec);

    // When: call createConenction
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);

    // Then: Connection が生成される。
    ASSERT_FALSE(pConnectionInternal1.isNull());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());

    // HTTPClientSession に、Proxy, Timeout, KeepAlive, KeepAliveTimeout が設定される。
    PocoHttpClientSessionPtr pPocoHttpClientSession = pConnectionInternal1->getPocoHttpClientSession();
    EXPECT_EQ(pProxy->getHost(), pPocoHttpClientSession->getProxyHost());
    EXPECT_EQ(pProxy->getPort(), pPocoHttpClientSession->getProxyPort());
    Poco::Timespan pocoTimeout = pPocoHttpClientSession->getTimeout();
    EXPECT_EQ(timeoutSec, pocoTimeout.seconds());
    EXPECT_TRUE(pPocoHttpClientSession->getKeepAlive());
    Poco::Timespan keepAliveTimeoutForPoco(1, 0, 0, static_cast<int>(keepAliveTimeoutSec), 0);
    EXPECT_EQ(keepAliveTimeoutForPoco, pPocoHttpClientSession->getKeepAliveTimeout());
}

// ConnectionPool が別のhostの Idle のConnectionがある場合の createConnection
// Connection が作成され ConnectionPool に追加される。
TEST(ConnectionPoolInternalUnitTest,
        createConnection_CreatesConnectionAndAddToConnectionPool_WhenOtherConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に host の違う Idle の Connection を追加。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // When: call createConnection
    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);

    // Then: 新しく Connection が生成される。
    ASSERT_FALSE(pConnectionInternal2.isNull());
    EXPECT_NE(pConnectionInternal1, pConnectionInternal2);
    ASSERT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
}

// 同じ host で Idle の再利用可能な Connection がある場合の createConnection
// Connection が作成され ConnectionPool に追加される。
TEST(ConnectionPoolInternalUnitTest,
        createConnection_CreatesConnectionAndAddToConnectionPool_WhenSameIdleConnectionExistInConnectinPool)
{
    // Given: ConnectionPool に Idle の Connection を追加する。
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    pConnectionPoolInternal->releaseConnection(pConnectionInternal1);
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // When: call createConnection
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);

    // Then: 新しく Connection が生成される。
    ASSERT_FALSE(pConnectionInternal2.isNull());
    EXPECT_NE(pConnectionInternal1, pConnectionInternal2);
    EXPECT_EQ(2, pConnectionPoolInternal->getTotalConnectionCount());
}

// scheme を ftp にして createConnection を呼び出す。
// HttpIllegalArgumentException が throw される。
TEST(ConnectionPoolInternalUnitTest,
        createConnection_ThrowsHttpIllegalArgumentException_WhenSchemeIsFtp)
{
    // Given: ConnectionPool is empty.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // When: call createConenction with ftp
    // Then: HttpIllegalArgumentException  が throw される。
    std::string url1 = "ftp://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    EASYHTTPCPP_EXPECT_THROW(pConnectionPoolInternal->createConnection(pRequest1, pEasyHttpContext),
            HttpIllegalArgumentException, 100700);
}

TEST(ConnectionPoolInternalUnitTest,
        createConnection_ThrowsHttpExecutionException_WhenUndecodableUrl)
{
    // Given: create ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    Request::Builder requestBuilder;
    Request::Ptr pMockRequest = new MockRequest(requestBuilder);
    EXPECT_CALL(*(static_cast<MockRequest*>(pMockRequest.get())), getUrl())
            .WillOnce(testing::ReturnRef(UnDecodableUrl));

    // When: call createConenction with undecodable url.
    // Then: HttpExecutionException  が throw される。
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pConnectionPoolInternal->createConnection(pMockRequest, pEasyHttpContext),
            HttpExecutionException, 100702);
}

// ConnectionPool が空の時の removeConnection。
// false が返る。
// ConnectionPool は空のまま。
TEST(ConnectionPoolInternalUnitTest, removeConnection_ReturnsFalse_WhenConnectionPoolIsEmpty)
{
    // Given: ConnectionPool is empty.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(NULL, url, pEasyHttpContext);

    // When: call removeConnection
    // Then: return false.
    EXPECT_FALSE(pConnectionPoolInternal->removeConnection(pConnectionInternal));
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に指定したConnection がない時の removeConnection。
// false が返る。
// ConnectionPool の状態は変わらない。
TEST(ConnectionPoolInternalUnitTest, removeConnection_ReturnsFalse_WhenConnectionNotExistInConnectionPool)
{
    // Given: add other Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);

    std::string url2 = "http://host02/path1";
    ConnectionInternal::Ptr pConnectionInternal2 = new ConnectionInternal(NULL, url2, pEasyHttpContext);

    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // When: call removeConnection
    // Then: return false.
    EXPECT_FALSE(pConnectionPoolInternal->removeConnection(pConnectionInternal2));
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に指定した Inuse の Connection がある時の removeConnection。
// true が返る。
// ConnectionPool から指定したConnection が削除される。
TEST(ConnectionPoolInternalUnitTest,
        removeConnection_ReturnsTrueAndRemoveConnectionFromConnectionPool_WhenInuseConnectionExistInConnectionPool)
{
    // Given: add target Inuse Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest, pEasyHttpContext);

    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // When: call removeConnection
    // Then: return true and remove Connection from ConnectionPool.
    EXPECT_TRUE(pConnectionPoolInternal->removeConnection(pConnectionInternal));
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に指定した Idle の Connection がある時の removeConnection。
// true が返る。
// ConnectionPool から指定したConnection が削除される。
TEST(ConnectionPoolInternalUnitTest,
        removeConnection_ReturnsTrueAndRemoveConnectionFromConnectionPool_WhenIdleConnectionExistInConnectionPool)
{
    // Given: add target Idle Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest, pEasyHttpContext);
    pConnectionPoolInternal->releaseConnection(pConnectionInternal);

    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // When: call removeConnection
    // Then: return true and remove Connection from ConnectionPool and timeoutTask was cancelled.
    EXPECT_TRUE(pConnectionPoolInternal->removeConnection(pConnectionInternal));
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に別の InuseのConnectionがあるときに、releaseConnection を呼び出す。
// falseが返る。
TEST(ConnectionPoolInternalUnitTest, releaseConnection_ReturnsFalse_WhenInuseConnectionNotExistInConnectionPool)
{
    // Given: add other Inuse Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());

    std::string url2 = "http://host02/path1";
    ConnectionInternal::Ptr pConnectionInternal2 = new ConnectionInternal(NULL, url2, pEasyHttpContext);

    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // When: call releaseConnection
    // Then: return false.
    EXPECT_FALSE(pConnectionPoolInternal->releaseConnection(pConnectionInternal2));
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool に指定した Inuse の Connection があるときに、releaseConnection を呼び出す。
// true が返る。
// Connection の status が Idle になる。
TEST(ConnectionPoolInternalUnitTest,
        releaseConnection_ReturnsTrueAndChangesStatusToIdle_WhenConnectionIsInuseInConnectionPool)
{
    // Given: add Inuse Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest, pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());

    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: call releaseConnection
    // Then: return true and change to Idle.
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool にある Idle の Connection に releaseConnection を呼び出す。
// false が返る。
// Connection は Idle のまま。
TEST(ConnectionPoolInternalUnitTest, releaseConnection_ReturnsFalse_WhenConnectionIsIdleInConnectionPool)
{
    // Given: add Idle Connection in ConnectionPool.
    ConnectionPoolInternal::Ptr pConnectionPoolInternal = ConnectionPool::createConnectionPool()
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url = "http://host01/path1";
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(url).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest, pEasyHttpContext);
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());

    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    EXPECT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: call releaseConnection
    // Then: return false.
    EXPECT_FALSE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    EXPECT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    EXPECT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
}

// idle 数が、max idle 数と同じ時に、存在する Inuse の Connection で releaseConnection を呼び出す。
// 一番古いIdle の Connection が ConnectionPool から削除される。
// idle 数が、max idle数になる。
TEST(ConnectionPoolInternalUnitTest, releaseConnection_RemovesOldestIdleConnection_WhenKeepAliveIdleCountIsMax)
{
    // Given: KeepAliveIdleContMax まで、Idle のConnection を Connection Pool に追加する。
    //        Inuse の Connection を ConnectionPool に追加する。
    unsigned int keepAliveIdleCountMax = 3;
    unsigned long keepAliveTimeoutSec = 20;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal1));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // Windows は時間取得の精度が低く、Timestamp で取得できる時間が 約15ms（クロック割り込み間隔に依存）なので、
    // 次の Connection update まで 20ms 待ちます.
    Poco::Thread::sleep(20);

    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal2));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal2->getStatus());

    Poco::Thread::sleep(20);

    std::string url3 = "http://host03/path1";
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    ConnectionInternal::Ptr pConnectionInternal3 = pConnectionPoolInternal->createConnection(pRequest3,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal3));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal3->getStatus());

    Poco::Thread::sleep(20);

    std::string url4 = "http://host04/path1";
    Request::Builder requestBuilder4;
    Request::Ptr pRequest4 = requestBuilder4.setUrl(url4).build();
    ConnectionInternal::Ptr pConnectionInternal4 = pConnectionPoolInternal->createConnection(pRequest4,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal4->getStatus());

    ASSERT_EQ(keepAliveIdleCountMax, pConnectionPoolInternal->getKeepAliveIdleCountMax());
    ASSERT_EQ(4, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(3, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: call releaseConnection
    // Then: return true. 一番古い Idle の Connection が remove される。
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal4));

    EXPECT_FALSE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal1));
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal4->getStatus());
    EXPECT_EQ(3, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    EXPECT_EQ(3, pConnectionPoolInternal->getTotalConnectionCount());
}

// idle 数が、max idle 数と同じ時に、一番古い Idle Connection を再利用して、releaseConnection する。
// その後、存在する Inuse の Connection で releaseConnection を呼び出す。
//
// 二番目に古い Idle のConnection が ConnectionPool から削除される。
// idle 数が、max idle数になる。
TEST(ConnectionPoolInternalUnitTest,
        releaseConnection_RemovesSecondOldestIdleConnection_WhenKeepAliveIdleCountIsMaxAndAfterReuseAndReleaseOldestConnection)
{
    // Given: KeepAliveIdleContMax まで、Idle のConnection を Connection Pool に追加する。
    //        Inuse の Connection を ConnectionPool に追加する。
    //        一番古い Idle のConnection を再利用したあと、再度 Idle にする。
    unsigned int keepAliveIdleCountMax = 3;
    unsigned long keepAliveTimeoutSec = 20;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal1));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal1->getStatus());

    // Windows は時間取得の精度が低く、Timestamp で取得できる時間が 約15ms（クロック割り込み間隔に依存）なので、
    // 次の Connection update まで 20ms 待ちます.
    Poco::Thread::sleep(20);

    std::string url2 = "http://host02/path1";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->createConnection(pRequest2,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal2));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal2->getStatus());

    Poco::Thread::sleep(20);

    std::string url3 = "http://host03/path1";
    Request::Builder requestBuilder3;
    Request::Ptr pRequest3 = requestBuilder3.setUrl(url3).build();
    ConnectionInternal::Ptr pConnectionInternal3 = pConnectionPoolInternal->createConnection(pRequest3,
            pEasyHttpContext);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal3));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal3->getStatus());

    Poco::Thread::sleep(20);

    std::string url4 = "http://host04/path1";
    Request::Builder requestBuilder4;
    Request::Ptr pRequest4 = requestBuilder4.setUrl(url4).build();
    ConnectionInternal::Ptr pConnectionInternal4 = pConnectionPoolInternal->createConnection(pRequest4,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal4->getStatus());

    Poco::Thread::sleep(20);

    // 一番古い Idle Connection を再利用して再度 Idle にする。
    bool connectionReused = false;
    ConnectionInternal::Ptr pConnectionInternal5 = pConnectionPoolInternal->getConnection(pRequest1, pEasyHttpContext,
            connectionReused);
    ASSERT_EQ(pConnectionInternal1, pConnectionInternal5);
    ASSERT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal5));
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal5->getStatus());
    
    ASSERT_EQ(keepAliveIdleCountMax, pConnectionPoolInternal->getKeepAliveIdleCountMax());
    ASSERT_EQ(4, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(3, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    Poco::Thread::sleep(20);

    // When: call releaseConnection
    // Then: return true. ２番目に古い Idle Connection が remove される。
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal4));

    EXPECT_FALSE(pConnectionPoolInternal->isConnectionExisting(pConnectionInternal2));
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal4->getStatus());
    EXPECT_EQ(3, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    EXPECT_EQ(3, pConnectionPoolInternal->getTotalConnectionCount());
}

// Connection が Inuse の時、releasesConnection を呼び出すと、Idle になって、KeepAliveTimeoutTask が登録される。
// KeepAliveTimeuotSec 経過すると、Connection は、削除される。
TEST(ConnectionPoolInternalUnitTest, releaseConnection_ReturnsTrueAndStartsKeepAliveTimerTask_WhenConnectionIsInuse)
{
    unsigned int keepAliveIdleCountMax = 10;
    unsigned long keepAliveTimeoutSec = 2;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: one Inuse Connection.
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    Poco::Timestamp startTime;

    // When: call releaseConnection
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask =
            pConnectionPoolInternal->getKeepAliveTimeoutTask(pConnectionInternal);
    EXPECT_FALSE(pKeepAliveTimeoutTask.isNull());

    // Then: expirationTime は、startTime + keepAliveTimeoutSec とほぼ同じ。
    //       expirationTime で、keepAliveTimeoutTask::run が呼び出される。

    // Windows では Poco::Util::Timer でのタイマーの精度が低い為、1500millisec のマージンを取ります.
    Poco::Timestamp minExpectedExpirationTime = startTime + (keepAliveTimeoutSec * 1000000 - 1500 * 1000);
    Poco::Timestamp maxExpectedExpirationTime = startTime + (keepAliveTimeoutSec * 1000000 + 1500 * 1000);

    // expirationTime
    Poco::Timestamp expirationTime = pKeepAliveTimeoutTask->getKeepAliveTimeoutExpirationTime();
    EXPECT_LE(startTime + keepAliveTimeoutSec, expirationTime); // startTime + keepAliveTimeoutSec より必ず大きい。
    EXPECT_GE(maxExpectedExpirationTime, expirationTime);

    // Windows では sleep をタイマ割り込みで制御しており、精度が保証されないので 5sec 待ちます.
    Poco::Thread::sleep(5 * 1000); // wait 5sec

    // keepAliveTimeoutCheckTask::run が呼び出される。
    Poco::Timestamp expiredTime = pKeepAliveTimeoutTask->lastExecution();
    EXPECT_LE(minExpectedExpirationTime, expiredTime);
    EXPECT_GE(maxExpectedExpirationTime, expiredTime);

    // connectionPool は、空になる。
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// cancel された Connection の releaseConnection は、removeConnection となる。
TEST(ConnectionPoolInternalUnitTest,
        releaseConnection_ReturnsFalseAndNotStartsKeepAliveTimerTask_WhenConnectionWasCancelled)
{
    unsigned int keepAliveIdleCountMax = 10;
    unsigned long keepAliveTimeoutSec = 2;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: Connection is cancelled.
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    pConnectionInternal->cancel();
    ASSERT_TRUE(pConnectionInternal->isCancelled());

    // When: call releaseConnection
    // Then: return false
    // not start KeepAliveTimeoutTask
    EXPECT_FALSE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask =
            pConnectionPoolInternal->getKeepAliveTimeoutTask(pConnectionInternal);
    EXPECT_TRUE(pKeepAliveTimeoutTask.isNull());

    // connectionPool は、空になる。
    EXPECT_EQ(0, pConnectionPoolInternal->getTotalConnectionCount());
}

// ConnectionPool にない KeepAliveTimeoutTask で、onKeepAliveTimeoutExpired を呼び出す。
// ConnectionPool に変化なし。
TEST(ConnectionPoolInternalUnitTest,
        onKeepAliveTimeoutExpired_DoesNotChangeConnectionPool_WhenKeepAliveTimeoutTaskNotInConnectionPool)
{
    unsigned int keepAliveIdleCountMax = 10;
    unsigned long keepAliveTimeoutSec = 2;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: start KeepAliveTimeoutTask
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // releaseConnection
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal));
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());

    // When: 待っていない KeepAliveTimeoutTask で onKeepAliveTimeoutExpired を呼び出す。
    Poco::Timestamp expirationTime;
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = new KeepAliveTimeoutTask(expirationTime, pConnectionPoolInternal);
    pConnectionPoolInternal->onKeepAliveTimeoutExpired(pKeepAliveTimeoutTask);

    // THen: ConnectionPool に変化なし。
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

// KeepAliveTimeout 待ちの時に、getConnection で Connection を再利用すると、KeepAliveTimeoutTask が cancel される。
TEST(ConnectionPoolInternalUnitTest,
        getConnection_ReusesConnectionAndCancelsKeepAliveTimeoutTask_WhenWaittKeepAliveTimeout)
{
    unsigned int keepAliveIdleCountMax = 10;
    unsigned long keepAliveTimeoutSec = 2;
    ConnectionPoolInternal::Ptr pConnectionPoolInternal =
            ConnectionPool::createConnectionPool(keepAliveIdleCountMax, keepAliveTimeoutSec)
            .unsafeCast<ConnectionPoolInternal>();

    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();

    // Given: start KeepAliveTimeoutTask
    std::string url1 = "http://host01/path1";
    Request::Builder requestBuilder1;
    Request::Ptr pRequest1 = requestBuilder1.setUrl(url1).build();
    ConnectionInternal::Ptr pConnectionInternal1 = pConnectionPoolInternal->createConnection(pRequest1,
            pEasyHttpContext);
    ASSERT_EQ(ConnectionInternal::Inuse, pConnectionInternal1->getStatus());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());

    // releaseConnection
    EXPECT_TRUE(pConnectionPoolInternal->releaseConnection(pConnectionInternal1));
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(1, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask =
            pConnectionPoolInternal->getKeepAliveTimeoutTask(pConnectionInternal1);
    ASSERT_FALSE(pKeepAliveTimeoutTask.isNull());

    // When: call getConnection with same server
    std::string url2 = "http://host01/path2";
    Request::Builder requestBuilder2;
    Request::Ptr pRequest2 = requestBuilder2.setUrl(url2).build();
    bool connectionReused2;
    ConnectionInternal::Ptr pConnectionInternal2 = pConnectionPoolInternal->getConnection(pRequest2,
            pEasyHttpContext, connectionReused2);

    // Then: reuse Connection and cancel KeepAliveTimeoutTask
    EXPECT_TRUE(connectionReused2);
    EXPECT_EQ(pConnectionInternal1, pConnectionInternal2);
    EXPECT_TRUE(pKeepAliveTimeoutTask->isCancelled());
    ASSERT_EQ(1, pConnectionPoolInternal->getTotalConnectionCount());
    ASSERT_EQ(0, pConnectionPoolInternal->getKeepAliveIdleConnectionCount());
}

} /* namespace test */
} /* namespace easyhttpcpp */
