/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_PARTIALMOCKSCHEDULEDFUTURETASK_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_PARTIALMOCKSCHEDULEDFUTURETASK_H_INCLUDED

#include "gmock/gmock.h"

#include "easyhttpcpp/executorservice/ScheduledFutureTask.h"

namespace easyhttpcpp {
namespace testutil {

template<class Result>
class PartialMockScheduledFutureTask : public executorservice::ScheduledFutureTask<Result> {
public:
    typedef Poco::AutoPtr<PartialMockScheduledFutureTask<Result> > Ptr;

    MOCK_METHOD0(runTask, void());
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_PARTIALMOCKSCHEDULEDFUTURETASK_H_INCLUDED */
