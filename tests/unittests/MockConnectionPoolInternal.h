/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONPOOLINTERNAL_H_INCLUDED
#define EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONPOOLINTERNAL_H_INCLUDED

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ConnectionPoolInternal.h"

namespace easyhttpcpp {
namespace test {

class MockConnectionPoolInternal : public ConnectionPoolInternal {
public:
    MockConnectionPoolInternal(unsigned int keepAliveIdleCountMax, unsigned long keepAliveTimeoutSec) :
    ConnectionPoolInternal(keepAliveIdleCountMax, keepAliveTimeoutSec)
    {
    }

    MOCK_CONST_METHOD0(getKeepAliveIdleCountMax, unsigned int());
    MOCK_CONST_METHOD0(getKeepAliveTimeoutSec, unsigned long());
    MOCK_METHOD0(getKeepAliveIdleConnectionCount, unsigned int());
    MOCK_METHOD0(getTotalConnectionCount, unsigned int());

    MOCK_METHOD3(getConnection, ConnectionInternal::Ptr(Request::Ptr pRequest, EasyHttpContext::Ptr pContext,
            bool& connectionReused));
    MOCK_METHOD2(createConnection, ConnectionInternal::Ptr(Request::Ptr pRequest, EasyHttpContext::Ptr pContext));
    MOCK_METHOD1(removeConnection, bool(ConnectionInternal::Ptr pConnectionInternal));
    MOCK_METHOD1(releaseConnection, bool(ConnectionInternal::Ptr pConnectionInternal));
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONPOOLINTERNAL_H_INCLUDED */
