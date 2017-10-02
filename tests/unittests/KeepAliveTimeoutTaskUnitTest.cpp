/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "KeepAliveTimeoutTask.h"
#include "MockKeepAliveTimeoutListener.h"

namespace easyhttpcpp {
namespace test {

TEST(KeepAliveTimeoutTaskUnitTest, getKeepAliveTimeoutExpirationTime_ReturnsExpirationTime_WhenSpecifiedByConstructor)
{
    // Given: specify expirationTime by constructor
    Poco::Timestamp expirationTime;
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = new KeepAliveTimeoutTask(expirationTime, NULL);

    // When: call getKeepAliveTimeoutExpirationTime
    // Then: return specified by constructor.
    EXPECT_EQ(expirationTime, pKeepAliveTimeoutTask->getKeepAliveTimeoutExpirationTime());
}

TEST(KeepAliveTimeoutTaskUnitTest, run_ExecutesKeepAliveTimeoutListener_WhenSpecifiedByConstructor)
{
    // Given: specify KeepAliveTimeoutListener by constructor
    Poco::Timestamp expirationTime;
    MockKeepAliveTimeoutListener mockListener;
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = new KeepAliveTimeoutTask(expirationTime, &mockListener);
    EXPECT_CALL(mockListener, onKeepAliveTimeoutExpired(pKeepAliveTimeoutTask.get())).Times(1);

    // When: call run
    // Then: be called onKeepAliveTimeoutExpired.
    pKeepAliveTimeoutTask->run();
}

TEST(KeepAliveTimeoutTaskUnitTest, run_EndsNormaly_WhenNotSpecifiedByConstructor)
{
    // Given: not specify KeepAliveTimeoutListener by constructor
    Poco::Timestamp expirationTime;
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = new KeepAliveTimeoutTask(expirationTime, NULL);

    // When: call run
    // Then: run ends normally.
    pKeepAliveTimeoutTask->run();
}

} /* namespace test */
} /* namespace easyhttpcpp */
