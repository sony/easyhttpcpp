/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/ConnectionPool.h"

#include "ConnectionPoolInternal.h"

namespace easyhttpcpp {

static const unsigned int DefaultMaxKeepAliveIdleCount = 10;
static const unsigned long DefaultKeepAliveTimeoutSec = 60;

ConnectionPool::Ptr ConnectionPool::createConnectionPool()
{
    return new ConnectionPoolInternal(DefaultMaxKeepAliveIdleCount, DefaultKeepAliveTimeoutSec);
}

ConnectionPool::Ptr ConnectionPool::createConnectionPool(unsigned int maxKeepAliveIdleCount,
            unsigned long keepAliveTimeoutSec)
{
    return new ConnectionPoolInternal(maxKeepAliveIdleCount, keepAliveTimeoutSec);
}

} /* namespace easyhttpcpp */
