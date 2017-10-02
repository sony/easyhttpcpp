/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONSTATUSLISTENER_H_INCLUDED
#define EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONSTATUSLISTENER_H_INCLUDED

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ConnectionStatusListener.h"

namespace easyhttpcpp {
namespace test {

class MockConnectionStatusListener : public ConnectionStatusListener {
public:
    MOCK_METHOD2(onIdle, void(ConnectionInternal* pConnectionInternal, bool& listenerInvalidated));
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_UNITTEST_MOCKCONNECTIONSTATUSLISTENER_H_INCLUDED */
