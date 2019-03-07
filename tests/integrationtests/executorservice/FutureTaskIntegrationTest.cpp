/*
 * Copyright 2017 Sony Corporation
 */

#include <Poco/Event.h>
#include <Poco/Thread.h>

#include "gtest/gtest.h"

#include "easyhttpcpp/executorservice/FutureTask.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"
#include "TestLogger.h"

namespace easyhttpcpp {
namespace executorservice {
namespace test {

namespace {

const std::string ResultString = "success!";

/**
 * Testee is a pure virtual class so subclass is needed to test it.
 */
class TestFutureTask : public FutureTask<std::string> {
public:
    typedef Poco::AutoPtr<TestFutureTask> Ptr;

    TestFutureTask(const std::string& resultString, long runBlockTimeMillis) : m_resultString(resultString),
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
 * Integration tests for FutureTask with QueuedThreadPool.
 */
class FutureTaskIntegrationTest : public testing::Test {
public:
    QueuedThreadPool::Ptr m_pThreadPool;

    virtual void SetUp()
    {
        m_pThreadPool = new QueuedThreadPool();

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    virtual void TearDown()
    {
        EASYHTTPCPP_TESTLOG_TEARDOWN_START();

        m_pThreadPool->shutdown();
    }
};

TEST_F(FutureTaskIntegrationTest, get_ReturnsResult_WhenResultWasSetInRunTask)
{
    // Given: result will be set successfully
    TestFutureTask::Ptr pTestee = new TestFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    m_pThreadPool->start(pTestee);
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

TEST_F(FutureTaskIntegrationTest, get_ThrowsFutureCancellationException_WhenCancelledWhileRunning)
{
    // Given: task is cancelled during running
    TestFutureTask::Ptr pTestee = new TestFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    m_pThreadPool->start(pTestee);
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

TEST_F(FutureTaskIntegrationTest, getWithTimeout_ReturnsResult_WhenResultWasSetInRunTask)
{
    // Given: result will be set successfully
    TestFutureTask::Ptr pTestee = new TestFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    m_pThreadPool->start(pTestee);
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

TEST_F(FutureTaskIntegrationTest, getWithTimeout_ThrowsFutureCancellationException_WhenCancelledWhileRunning)
{
    // Given: task is cancelled during running
    TestFutureTask::Ptr pTestee = new TestFutureTask(ResultString, 1000);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    m_pThreadPool->start(pTestee);
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

TEST_F(FutureTaskIntegrationTest, getWithTimeout_ThrowsFutureTimeoutException_WhenTaskDoesNotFinishInTime)
{
    // Given: task will consume long time
    TestFutureTask::Ptr pTestee = new TestFutureTask(ResultString, 2000);

    m_pThreadPool->start(pTestee);
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

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
