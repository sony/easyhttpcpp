/*
 * Copyright 2017 Sony Corporation
 */

#include <Poco/Event.h>
#include <Poco/Thread.h>
#include <Poco/Util/Timer.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/executorservice/ScheduledFutureTask.h"

namespace easyhttpcpp {
namespace executorservice {
namespace test {

namespace {

const std::string ResultString = "success!";

/**
 * Testee is a pure virtual class so subclass is needed to test it.
 */
class TestScheduledFutureTask : public ScheduledFutureTask<std::string> {
public:
    typedef Poco::AutoPtr<TestScheduledFutureTask> Ptr;

    TestScheduledFutureTask(const std::string& resultString, long runBlockTimeMillis) : m_resultString(resultString),
    m_runBlockTimeMillis(runBlockTimeMillis), m_startWaiter(false)
    {
    }

    virtual void runTask()
    {
        m_startWaiter.set();
        Poco::Thread::sleep(m_runBlockTimeMillis);
        setResult(m_resultString);
    }

    void waitToStart(long milliseconds)
    {
        m_startWaiter.wait(milliseconds);
    }

private:
    std::string m_resultString;
    long m_runBlockTimeMillis;
    Poco::Event m_startWaiter;
};

} /* namespace */

/**
 * Integration tests for ScheduledFutureTask with Timer.
 */
class ScheduledFutureTaskIntegrationTest : public testing::Test {
public:
    Poco::Util::Timer m_pocoTimer;
};

TEST_F(ScheduledFutureTaskIntegrationTest, get_ReturnsResult_WhenResultWasSetInRunTask)
{
    // Given: result will be set successfully
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    m_pocoTimer.schedule(pTestee, now);
    pTestee->waitToStart(1000);

    // not yet done
    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // When: call get()
    // Then: result is returned
    EXPECT_EQ(ResultString, pTestee->get());

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TEST_F(ScheduledFutureTaskIntegrationTest, get_ThrowsFutureCancellationException_WhenCancelledWhileRunning)
{
    // Given: task is cancelled during running
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    m_pocoTimer.schedule(pTestee, now);
    pTestee->waitToStart(1000);

    ASSERT_TRUE(pTestee->cancel(true));

    // isCancelled() becomes true immediately
    ASSERT_TRUE(pTestee->isCancelled());
    ASSERT_FALSE(pTestee->isDone());

    // When: call get()
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        // Then: exception is thrown
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TEST_F(ScheduledFutureTaskIntegrationTest, getWithTimeout_ReturnsResult_WhenResultWasSetInRunTask)
{
    // Given: result will be set successfully
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    m_pocoTimer.schedule(pTestee, now);
    pTestee->waitToStart(1000);

    // not yet done
    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // When: call get(timeout)
    // Then: result is returned
    EXPECT_EQ(ResultString, pTestee->get(10 * 1000));

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TEST_F(ScheduledFutureTaskIntegrationTest, getWithTimeout_ThrowsFutureCancellationException_WhenCancelledWhileRunning)
{
    // Given: task is cancelled during running
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    m_pocoTimer.schedule(pTestee, now);
    pTestee->waitToStart(1000);

    ASSERT_TRUE(pTestee->cancel(true));

    // isCancelled() becomes true immediately
    ASSERT_TRUE(pTestee->isCancelled());
    ASSERT_FALSE(pTestee->isDone());

    // When: call get(timeout)
    try {
        pTestee->get(10 * 1000);
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        // Then: exception is thrown
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TEST_F(ScheduledFutureTaskIntegrationTest, getWithTimeout_ThrowsFutureTimeoutException_WhenTaskDoesNotFinishInTime)
{
    // Given: task will consume long time
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 2000);

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    m_pocoTimer.schedule(pTestee, now);
    pTestee->waitToStart(1000);

    // When: call get(timeout) with smaller timeout
    try {
        pTestee->get(10);
        FAIL() << "Expected to throw FutureTimeoutException";
    } catch (const common::FutureTimeoutException& expected) {
        // Then: exception is thrown
    }

    // result is returned
    EXPECT_EQ(ResultString, pTestee->get());

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TEST_F(ScheduledFutureTaskIntegrationTest, getWithTimeout_ReturnsResultOnRetry_WhenTaskIsScheduledWithDelay)
{
    // Given: schedule task with delay
    TestScheduledFutureTask::Ptr pTestee = new TestScheduledFutureTask(ResultString, 0);

    pTestee.duplicate(); // run will release
    Poco::Timestamp now;
    Poco::Timespan delaySpan(3, 0);
    Poco::Timestamp expectedExecutionTime = now + delaySpan;
    m_pocoTimer.schedule(pTestee, expectedExecutionTime);

    // call get(timeout) with smaller timeout
    try {
        pTestee->get(10);
        FAIL() << "Expected to throw FutureTimeoutException";
    } catch (const common::FutureTimeoutException& expected) {
        // exception is thrown
    }

    // not yet executed
    EXPECT_EQ(Poco::Timestamp::fromEpochTime(0), pTestee->lastExecution());
    EXPECT_FALSE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());

    // When: call get(timeout) again with delay + margin
    // Then: result is returned

    // Windows では Poco::Util::Timer でのタイマーの精度が低い為、2sec のマージンを取ります.
    Poco::Timespan marginSpan(2, 0);
    EXPECT_EQ(ResultString, pTestee->get(delaySpan.totalMilliseconds() + marginSpan.totalMilliseconds()));

    // executed at expected time
    EXPECT_THAT(pTestee->lastExecution(), testing::AllOf(
            testing::Ge(expectedExecutionTime - marginSpan), testing::Le(expectedExecutionTime + marginSpan)));
    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
