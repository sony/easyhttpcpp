/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_PARTIALMOCKFUTURETASK_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_PARTIALMOCKFUTURETASK_H_INCLUDED

#include "gmock/gmock.h"

#include "easyhttpcpp/executorservice/FutureTask.h"

namespace easyhttpcpp {
namespace testutil {

template<class Result>
class PartialMockFutureTask : public executorservice::FutureTask<Result> {
public:
    typedef Poco::AutoPtr<PartialMockFutureTask<Result> > Ptr;

    MOCK_METHOD0(runTask, void());
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_PARTIALMOCKFUTURETASK_H_INCLUDED */
