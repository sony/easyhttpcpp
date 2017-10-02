/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_UNITTEST_MOCKKEEPALIVETIMEOUTLISTENER_H_INCLUDED
#define EASYHTTPCPP_TEST_UNITTEST_MOCKKEEPALIVETIMEOUTLISTENER_H_INCLUDED

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "KeepAliveTimeoutListener.h"

namespace easyhttpcpp {
namespace test {

class MockKeepAliveTimeoutListener : public KeepAliveTimeoutListener {
public:
    MOCK_METHOD1(onKeepAliveTimeoutExpired, void(const KeepAliveTimeoutTask* pKeepAliveTimeoutTask));
};


} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_UNITTEST_MOCKKEEPALIVETIMEOUTLISTENER_H_INCLUDED */
