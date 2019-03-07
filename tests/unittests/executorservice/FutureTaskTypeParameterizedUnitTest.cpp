/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/executorservice/FutureTask.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"
#include "easyhttpcpp/executorservice/ScheduledFutureTask.h"
#include "PartialMockFutureTask.h"
#include "PartialMockScheduledFutureTask.h"

/**
 * Type-parameterized unit tests for FutureTask and ScheduledFutureTask.
 */
namespace easyhttpcpp {
namespace executorservice {
namespace test {
namespace {

const long WaitTimeoutMillis = 10 * 1000;
const std::string ResultString = "success!";
const unsigned int ThreadPoolSize = 5;

/**
 * Implementation of FutureTask for testing.
 */
class TestFutureTask : public FutureTask<std::string> {
public:

    TestFutureTask(const std::string& resultString) : m_resultString(resultString)
    {
    }

    virtual void runTask()
    {
        setResult(m_resultString);
    }

private:
    std::string m_resultString;
};

/**
 * Implementation of ScheduledFutureTask for testing.
 */
class TestScheduledFutureTask : public ScheduledFutureTask<std::string> {
public:

    TestScheduledFutureTask(const std::string& resultString) : m_resultString(resultString)
    {
    }

    virtual void runTask()
    {
        setResult(m_resultString);
    }

private:
    std::string m_resultString;
};

/**
 * A task to call the testee on multiple threads. This is NOT testee. Yes, it's a bit confusing.
 */
class TaskResultGetter : public FutureTask<std::string> {
public:
    typedef Poco::AutoPtr<TaskResultGetter> Ptr;

    TaskResultGetter(common::Future<std::string>& testeeFuture, long timeoutMillis = 0) :
    m_testeeFuture(testeeFuture), m_timeoutMillis(timeoutMillis)
    {
    }

    virtual void runTask()
    {
        if (m_timeoutMillis > 0) {
            setResult(m_testeeFuture.get(m_timeoutMillis));
        } else {
            setResult(m_testeeFuture.get());
        }
    }

private:
    common::Future<std::string>& m_testeeFuture;
    long m_timeoutMillis;
};

template <typename T>
void callRun(T pTestee)
{
    // must be duplicated since task will release itself in run()
    pTestee.duplicate();

    // upcast is needed to call run() since it's protected in subclass
    static_cast<Poco::Runnable*> (pTestee.get())->run();
}

} /* namespace */

/**
 * fixture for type-parameterized test with subclass of testee, which is pure virtual
 * so subclass is needed to execute it.
 */
template <typename TargetClass>
class FutureTaskSubClassParameterizedUnitTest : public testing::Test {
public:
    QueuedThreadPool::Ptr m_pThreadPool;

    virtual void SetUp()
    {
        m_pThreadPool = new QueuedThreadPool(ThreadPoolSize, ThreadPoolSize);
    }

    virtual void TearDown()
    {
        m_pThreadPool->shutdown();
    }
};
TYPED_TEST_CASE_P(FutureTaskSubClassParameterizedUnitTest);

/**
 * fixture for type-parameterized test with partial mock of testee, which is useful to throw exception.
 */
template <typename TargetClass>
class FutureTaskMockParameterizedUnitTest : public testing::Test {
};
TYPED_TEST_CASE_P(FutureTaskMockParameterizedUnitTest);

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, get_ReturnsResultEachTime_WhenResultWasSetInRunTask)
{
    // Given: runTask() will set result
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get() twice
    // Then: result is returned each time
    EXPECT_EQ(ResultString, pTestee->get());
    EXPECT_EQ(ResultString, pTestee->get());

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest,
        get_ThrowsFutureExecutionExceptionEachTime_WhenEasyHttpExceptionWasThrownInRunTask)
{
    // Given: runTask() will throw EasyHttp exception, say, ExecutorServiceExecutionException
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw(ExecutorServiceExecutionException("thrown by test")));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get() twice
    // Then: exception is thrown with cause each time
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest, get_ThrowsFutureExecutionException_WhenStdExceptionWasThrownInRunTask)
{
    // Given: runTask() will throw std exception, say, runtime_error
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw(std::runtime_error("thrown by test")));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get()
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        // Then: exception is thrown with nested cause
        common::BaseException::Ptr pCause = expected.getCause();
        ASSERT_FALSE(pCause.cast<ExecutorServiceExecutionException>().isNull());
        EXPECT_FALSE(pCause->getCause().cast<common::StdException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest,
        get_ThrowsFutureExecutionException_WhenUnknownObjectWasThrownInRunTask)
{
    // Given: runTask() will throw something, say, char*
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw("so wicked to throw such a thing"));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get()
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        // Then: exception is thrown with cause
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        get_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithTrue)
{
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // Given: future task is cancelled with true
    ASSERT_TRUE(pTestee->cancel(true));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_TRUE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get() twice
    // Then: exception is thrown each time
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        get_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithFalse)
{
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // Given: future task is cancelled with false
    ASSERT_TRUE(pTestee->cancel(false));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_TRUE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get() twice
    // Then: exception is thrown each time
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }
    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, get_ReturnsResultEachTime_WhenCalledOnMultipleThreads)
{
    // Given: runTask() will set result
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    // When: get() is called on multiple threads
    TaskResultGetter::Ptr taskResultGetters[ThreadPoolSize];
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        taskResultGetters[i] = new TaskResultGetter(*pTestee);
        this->m_pThreadPool->start(taskResultGetters[i]);
    }

    // wait for worker threads to start
    Poco::Thread::sleep(100);

    callRun(pTestee);

    // Then: result is returned on each thread
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        EXPECT_EQ(ResultString, taskResultGetters[i]->get(WaitTimeoutMillis));
    }
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, getWithTimeout_ReturnsResultEachTime_WhenResultWasSetInRunTask)
{
    // Given: runTask() will set result
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout) twice
    // Then: result is returned each time
    EXPECT_EQ(ResultString, pTestee->get(WaitTimeoutMillis));
    EXPECT_EQ(ResultString, pTestee->get(WaitTimeoutMillis));

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest,
        getWithTimeout_ThrowsFutureExecutionExceptionEachTime_WhenEasyHttpExceptionWasThrownInRunTask)
{
    // Given: runTask() will throw EasyHttp exception, say, ExecutorServiceExecutionException
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw(ExecutorServiceExecutionException("thrown by test")));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout) twice
    // Then: exception is thrown with cause each time
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest,
        getWithTimeout_ThrowsFutureExecutionException_WhenStdExceptionWasThrownInRunTask)
{
    // Given: runTask() will throw std exception, say, runtime_error
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw(std::runtime_error("thrown by test")));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout)
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        // Then: exception is thrown with nested cause
        common::BaseException::Ptr pCause = expected.getCause();
        ASSERT_FALSE(pCause.cast<ExecutorServiceExecutionException>().isNull());
        EXPECT_FALSE(pCause->getCause().cast<common::StdException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskMockParameterizedUnitTest,
        getWithTimeout_ThrowsFutureExecutionException_WhenUnknownObjectWasThrownInRunTask)
{
    // Given: runTask() will throw something, say, char*
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam();
    EXPECT_CALL(*pTestee, runTask()).WillOnce(testing::Throw("so wicked to throw such a thing"));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout)
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureExecutionException";
    } catch (const common::FutureExecutionException& expected) {
        // Then: exception is thrown with cause
        EXPECT_FALSE(expected.getCause().cast<ExecutorServiceExecutionException>().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        getWithTimeout_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithTrue)
{
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // Given: future task is cancelled with true
    ASSERT_TRUE(pTestee->cancel(true));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_TRUE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout) twice
    // Then: exception is thrown each time
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        getWithTimeout_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithFalse)
{
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_FALSE(pTestee->isCancelled());

    // Given: future task is cancelled with false
    ASSERT_TRUE(pTestee->cancel(false));

    ASSERT_FALSE(pTestee->isDone());
    ASSERT_TRUE(pTestee->isCancelled());

    callRun(pTestee);

    // When: call get(timeout) twice
    // Then: exception is thrown each time
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }
    try {
        pTestee->get(WaitTimeoutMillis);
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        EXPECT_TRUE(expected.getCause().isNull());
    }

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_TRUE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, getWithTimeout_ReturnsResultEachTime_WhenCalledOnMultipleThreads)
{
    // Given: runTask() will set result
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    // When: get(timeout) is called on multiple threads
    TaskResultGetter::Ptr taskResultGetters[ThreadPoolSize];
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        taskResultGetters[i] = new TaskResultGetter(*pTestee, WaitTimeoutMillis);
        this->m_pThreadPool->start(taskResultGetters[i]);
    }

    // wait for worker threads to start
    Poco::Thread::sleep(100);

    callRun(pTestee);

    // Then: result is returned on each thread
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        EXPECT_EQ(ResultString, taskResultGetters[i]->get(WaitTimeoutMillis));
    }
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        getWithTimeout_ThrowsFutureTimeoutException_WhenTaskDoesNotFinishInTime)
{
    // Given: task will not finish in time; we will not call run()
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    // When: call get(timeout) with smaller timeout
    try {
        pTestee->get(10);
        FAIL() << "Expected to throw FutureTimeoutException";
    } catch (const common::FutureTimeoutException& expected) {
        // Then: exception is thrown with cause
        EXPECT_FALSE(expected.getCause().cast<common::PocoException>().isNull());
    }

    EXPECT_FALSE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, getWithTimeout_ReturnsResultOnRetry_WhenTaskDoesNotFinishInTime)
{
    // Given: task will not finish in time; we will not call run()
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    // call get(timeout) with smaller timeout
    try {
        pTestee->get(10);
        FAIL() << "Expected to throw FutureTimeoutException";
    } catch (const common::FutureTimeoutException& expected) {
        ASSERT_FALSE(expected.getCause().cast<common::PocoException>().isNull());
    }

    // let the task finish
    callRun(pTestee);

    // When: call get(timeout) again
    // Then: result is returned
    EXPECT_EQ(ResultString, pTestee->get(WaitTimeoutMillis));

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        getWithTimeout_ThrowsFutureTimeoutException_WhenCalledOnMultipleThreadsAndTaskDoesNotFinishInTime)
{
    // Given: task will not finish in time; we will not call run()
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    // When: get(timeout) is called on multiple threads
    TaskResultGetter::Ptr taskResultGetters[ThreadPoolSize];
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        taskResultGetters[i] = new TaskResultGetter(*pTestee, 10);
        this->m_pThreadPool->start(taskResultGetters[i]);
    }

    // wait for worker threads to start
    Poco::Thread::sleep(100);

    // Then: exception is thrown on each thread
    for (unsigned int i = 0; i < ThreadPoolSize; ++i) {
        try {
            taskResultGetters[i]->get(WaitTimeoutMillis);
            FAIL() << "Expected to throw FutureExecutionException";
        } catch (const common::FutureExecutionException& expected) {
            // cause is what testee threw
            EXPECT_FALSE(expected.getCause().cast<common::FutureTimeoutException>().isNull());
        }
    }
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest, cancel_ReturnsTrue_WhenCalledAfterTaskIsFinished)
{
    // Given: task is executed and finished
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    callRun(pTestee);

    EXPECT_EQ(ResultString, pTestee->get());

    EXPECT_TRUE(pTestee->isDone());
    EXPECT_FALSE(pTestee->isCancelled());

    // When: call cancel() with true and false
    // Then: true is returned and isCancelled() becomes true
    EXPECT_TRUE(pTestee->cancel(false));
    EXPECT_TRUE(pTestee->isCancelled());
    EXPECT_TRUE(pTestee->cancel(true));
    EXPECT_TRUE(pTestee->isCancelled());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        isDone_ReturnsExpectedValue_WhenCalledBeforeAndAfterExecutionOfTask)
{
    // Given: task will be executed successfully
    // When: call isDone()
    // Then: expected value is returned each time
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    EXPECT_FALSE(pTestee->isDone());

    callRun(pTestee);

    EXPECT_EQ(ResultString, pTestee->get());

    EXPECT_TRUE(pTestee->isDone());
}

TYPED_TEST_P(FutureTaskSubClassParameterizedUnitTest,
        isCancelled_ReturnsExpectedValue_WhenCalledBeforeAndAfterCancellationOfTask)
{
    // Given: task will be cancelled
    // When: call isCancelled()
    // Then: expected value is returned each time
    Poco::AutoPtr<TypeParam> pTestee = new TypeParam(ResultString);

    EXPECT_FALSE(pTestee->isCancelled());

    EXPECT_TRUE(pTestee->cancel(true));

    EXPECT_TRUE(pTestee->isCancelled());

    callRun(pTestee);

    try {
        pTestee->get();
        FAIL() << "Expected to throw FutureCancellationException";
    } catch (const common::FutureCancellationException& expected) {
        // exception is thrown
    }

    EXPECT_TRUE(pTestee->isCancelled());
}

// Now the tricky part:
// you need to register all test patterns using the REGISTER_TYPED_TEST_CASE_P macro before you can instantiate them.
// The first argument of the macro is the test case name; the rest are the names of the tests in this test case:
REGISTER_TYPED_TEST_CASE_P(FutureTaskSubClassParameterizedUnitTest,
        get_ReturnsResultEachTime_WhenResultWasSetInRunTask,
        get_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithTrue,
        get_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithFalse,
        get_ReturnsResultEachTime_WhenCalledOnMultipleThreads,
        getWithTimeout_ReturnsResultEachTime_WhenResultWasSetInRunTask,
        getWithTimeout_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithFalse,
        getWithTimeout_ThrowsFutureCancellationExceptionEachTime_WhenCancelledWithTrue,
        getWithTimeout_ReturnsResultEachTime_WhenCalledOnMultipleThreads,
        getWithTimeout_ThrowsFutureTimeoutException_WhenTaskDoesNotFinishInTime,
        getWithTimeout_ReturnsResultOnRetry_WhenTaskDoesNotFinishInTime,
        getWithTimeout_ThrowsFutureTimeoutException_WhenCalledOnMultipleThreadsAndTaskDoesNotFinishInTime,
        cancel_ReturnsTrue_WhenCalledAfterTaskIsFinished,
        isDone_ReturnsExpectedValue_WhenCalledBeforeAndAfterExecutionOfTask,
        isCancelled_ReturnsExpectedValue_WhenCalledBeforeAndAfterCancellationOfTask);
REGISTER_TYPED_TEST_CASE_P(FutureTaskMockParameterizedUnitTest,
        get_ThrowsFutureExecutionExceptionEachTime_WhenEasyHttpExceptionWasThrownInRunTask,
        get_ThrowsFutureExecutionException_WhenStdExceptionWasThrownInRunTask,
        get_ThrowsFutureExecutionException_WhenUnknownObjectWasThrownInRunTask,
        getWithTimeout_ThrowsFutureExecutionExceptionEachTime_WhenEasyHttpExceptionWasThrownInRunTask,
        getWithTimeout_ThrowsFutureExecutionException_WhenStdExceptionWasThrownInRunTask,
        getWithTimeout_ThrowsFutureExecutionException_WhenUnknownObjectWasThrownInRunTask);

// Finally, you are free to instantiate the pattern with the types you want.
// The typedef is necessary for the INSTANTIATE_TYPED_TEST_CASE_P macro to parse correctly.
// Otherwise the compiler will think that each comma in the type list introduces a new macro argument.
typedef testing::Types<TestFutureTask, TestScheduledFutureTask> FutureTaskSubClassTypes;
typedef testing::Types<testutil::PartialMockFutureTask<std::string>,
testutil::PartialMockScheduledFutureTask<std::string> > FutureTaskMockTypes;

// To distinguish different instances of the pattern, the first argument to the INSTANTIATE_TYPED_TEST_CASE_P macro is
// a prefix that will be added to the actual test case name. Remember to pick unique prefixes for different instances.
INSTANTIATE_TYPED_TEST_CASE_P(SubClass, FutureTaskSubClassParameterizedUnitTest, FutureTaskSubClassTypes);
INSTANTIATE_TYPED_TEST_CASE_P(Mock, FutureTaskMockParameterizedUnitTest, FutureTaskMockTypes);

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
