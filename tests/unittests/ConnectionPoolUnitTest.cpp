/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/ConnectionPool.h"

namespace easyhttpcpp {
namespace test {

static const unsigned int DefaultKeepAliveIdleContMax = 10;
static const unsigned long DefaultKeepAliveTimeouteSec = 60;

// パラメータなし
// デフォルト値で ConnectionPoolInternal が生成される。
TEST(ConnectionPoolUnitTest, createConnectionPool_createsConnectionPoolByDefaultValue_WhenWithoutParameters)
{
    // Given: none

    // When: createConnectionPool without parameter.
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();

    // Then: create ConnectionPool by default value
    ASSERT_FALSE(pConnectionPool.isNull());
    EXPECT_EQ(DefaultKeepAliveIdleContMax, pConnectionPool->getKeepAliveIdleCountMax());
    EXPECT_EQ(DefaultKeepAliveTimeouteSec, pConnectionPool->getKeepAliveTimeoutSec());
}

// パラメータあり
// 指定した値で ConnectionPoolInternal が生成される。
TEST(ConnectionPoolUnitTest, createConnectionPool_createsConnectionPoolBySpecifiedValue_WhenWithParameters)
{
    // Given: none

    // When: createConnectionPool with parameter.
    unsigned int keepAliveIdleCountMax = 5;
    unsigned long keepAliveTimeoutSec = 20;
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool(keepAliveIdleCountMax,
            keepAliveTimeoutSec);

    // Then: create ConnectionPool by specified value
    ASSERT_FALSE(pConnectionPool.isNull());
    EXPECT_EQ(keepAliveIdleCountMax, pConnectionPool->getKeepAliveIdleCountMax());
    EXPECT_EQ(keepAliveTimeoutSec, pConnectionPool->getKeepAliveTimeoutSec());
}

} /* namespace test */
} /* namespace easyhttpcpp */
