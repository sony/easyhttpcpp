/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKFUTURE_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKFUTURE_H_INCLUDED

#include "gmock/gmock.h"

#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/Future.h"

namespace easyhttpcpp {
namespace testutil {

template<class Result>
class MockFuture : public common::Future<Result> {
public:
    typedef Poco::SharedPtr<MockFuture <Result> > Ptr;

    MOCK_METHOD1_T(cancel, bool(bool mayInterruptIfRunning));
    MOCK_CONST_METHOD0_T(isCancelled, bool());
    MOCK_CONST_METHOD0_T(isDone, bool());
    MOCK_METHOD0_T(get, Result());
    MOCK_METHOD1_T(get, Result(unsigned long timeoutMillis));
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKFUTURE_H_INCLUDED */
