/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKINTERCEPTOR_H_INCLUDED

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/Interceptor.h"

namespace easyhttpcpp {
namespace testutil {

class MockInterceptor : public easyhttpcpp::Interceptor {
public:
    typedef Poco::AutoPtr<MockInterceptor> Ptr;

    MOCK_METHOD1(intercept, easyhttpcpp::Response::Ptr(Interceptor::Chain& chain));
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKINTERCEPTOR_H_INCLUDED */
