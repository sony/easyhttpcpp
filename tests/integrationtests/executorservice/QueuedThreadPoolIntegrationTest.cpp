/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/AtomicCounter.h"
#include "Poco/Random.h"

#include "easyhttpcpp/executorservice/BoundBlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"
#include "easyhttpcpp/executorservice/FutureTask.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"
#include "easyhttpcpp/executorservice/Task.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

namespace easyhttpcpp {
namespace executorservice {
namespace test {

class QueuedThreadPoolIntegrationTest : public testing::Test {
protected:

    Poco::SharedPtr<Poco::Event> m_pMainThreadBlocker;
    Poco::SharedPtr<Poco::Event> m_pWorkerThreadBlocker;
    int m_threadCount;
    Poco::AtomicCounter m_runCount;
    Poco::AtomicCounter m_pushCount;

    void SetUp()
    {
        m_threadCount = 1;
        m_runCount = 0;
        m_pMainThreadBlocker = new Poco::Event();
        m_pWorkerThreadBlocker = new Poco::Event(false);
        m_pBoundBlockingQueue = new TestBoundBlockingQueue(this);
        m_pushCount = 0;

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    class TestTask : public Task {
    public:
        QueuedThreadPoolIntegrationTest* m_testClass;
        unsigned int m_sleepTime;

        TestTask(QueuedThreadPoolIntegrationTest* testClass)
        {
            m_testClass = testClass;
            m_sleepTime = 1; // 1ms
        }

        TestTask(QueuedThreadPoolIntegrationTest* testClass, unsigned int sleepTime)
        {
            m_testClass = testClass;
            m_sleepTime = sleepTime;
        }

        void runTask()
        {
            // ThreadPoolを溜めるために少しsleep
            Poco::Thread::sleep(m_sleepTime);

            // Taskが全て呼び出されたらwaitを解除
            m_testClass->m_runCount++;
            if (m_testClass->m_threadCount == m_testClass->m_runCount.value()) {
                m_testClass->m_pMainThreadBlocker->set();
            }
        }
    };

    class WaitTask : public Task {
    public:
        QueuedThreadPoolIntegrationTest* m_testClass;
        unsigned int m_waitTime;

        WaitTask(QueuedThreadPoolIntegrationTest* testClass, unsigned int waitTime)
        {
            m_testClass = testClass;
            m_waitTime = waitTime;
        }

        void runTask()
        {
            m_testClass->m_pWorkerThreadBlocker->wait(m_waitTime);

            // Taskが全て呼び出されたらwaitを解除
            m_testClass->m_runCount++;
            if (m_testClass->m_threadCount == m_testClass->m_runCount.value()) {
                m_testClass->m_pMainThreadBlocker->set();
            }
        }
    };

    class QueuedThreadPoolStartTask : public Task {
    public:
        QueuedThreadPoolIntegrationTest* m_testClass;
        unsigned int m_sleepTime;
        QueuedThreadPool::Ptr m_queuedThreadPool;
        bool m_wait;

        QueuedThreadPoolStartTask(QueuedThreadPoolIntegrationTest* testClass, QueuedThreadPool::Ptr queuedThreadPool,
                unsigned int sleepTime)
        {
            m_testClass = testClass;
            m_queuedThreadPool = queuedThreadPool;
            m_sleepTime = sleepTime;
            m_wait = false;
        }

        QueuedThreadPoolStartTask(QueuedThreadPoolIntegrationTest* testClass, QueuedThreadPool::Ptr queuedThreadPool,
                bool wait)
        {
            m_testClass = testClass;
            m_queuedThreadPool = queuedThreadPool;
            m_sleepTime = 0;
            m_wait = wait;
        }

        void runTask()
        {
            if (m_wait) {
                m_testClass->m_pWorkerThreadBlocker->wait();
            } else {
                Poco::Thread::sleep(m_sleepTime);
            }
            Poco::Random random;
            Task::Ptr pTestTask = new TestTask(m_testClass, random.next(1000));
            m_queuedThreadPool->start(pTestTask);
        }
    };

    class TestFutureTask : public FutureTask<std::string> {
    public:
        typedef Poco::AutoPtr<TestFutureTask> Ptr;

        TestFutureTask(long runBlockTimeMillis) : m_runBlockTimeMillis(runBlockTimeMillis)
        {
        }

        virtual void runTask()
        {
            Poco::Thread::sleep(m_runBlockTimeMillis);
            setResult("TestFutureTask is done");
        }

    private:
        long m_runBlockTimeMillis;
    };

    class TestBoundBlockingQueue : public BoundBlockingQueue {
    public:
        typedef Poco::AutoPtr<TestBoundBlockingQueue> Ptr;
        QueuedThreadPoolIntegrationTest* m_testClass;

        TestBoundBlockingQueue(QueuedThreadPoolIntegrationTest* testClass)
        {
            m_testClass = testClass;
        }

        bool push(Task::Ptr pTask)
        {
            bool ret = BoundBlockingQueue::push(pTask);
            if (ret) {
                m_testClass->m_pushCount++;
            }
            return ret;
        }

        size_t getQueueSize()
        {
            return m_workerQueue.size();
        }
    };

    TestBoundBlockingQueue::Ptr m_pBoundBlockingQueue;
};

/** TaskにNULLを指定してstartを呼び出すとExceptionがthrowされる */
TEST_F(QueuedThreadPoolIntegrationTest, start_ThrowException_WhenTaskIsNull)
{
    // Given: QueuedThreadPoolを生成
    QueuedThreadPool queuedThreadPool;

    // When: TaskにNULLを指定してstartを呼び出す
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    EASYHTTPCPP_EXPECT_THROW(queuedThreadPool.start(NULL), ExecutorServiceIllegalArgumentException, 100600);
}

/** shutdownを呼び出した後に、startを呼び出すとExceptionがthrowされる */
TEST_F(QueuedThreadPoolIntegrationTest, start_ThrowException_WhenAfterCallShutdown)
{
    // Given: QueuedThreadPoolを生成し、shutdownを呼び出す
    QueuedThreadPool queuedThreadPool;
    queuedThreadPool.shutdown();

    // When: startを呼び出す
    // Then: ExecutorServiceIllegalStateExceptionがthrowされる
    Task::Ptr pTestTask = new TestTask(this);
    EASYHTTPCPP_EXPECT_THROW(queuedThreadPool.start(pTestTask), ExecutorServiceIllegalStateException, 100601);
}

TEST_F(QueuedThreadPoolIntegrationTest,
        start_ThrowsExecutorServiceIllegalStateException_WhenCalledAfterShutdownAndJoinAll)
{
    // Given: QueuedThreadPoolを生成し、shutdownAndJoinAllを呼び出す
    QueuedThreadPool queuedThreadPool;
    queuedThreadPool.shutdownAndJoinAll();

    // When: startを呼び出す
    // Then: ExecutorServiceIllegalStateExceptionがthrowされる
    Task::Ptr pTestTask = new TestTask(this);
    EASYHTTPCPP_EXPECT_THROW(queuedThreadPool.start(pTestTask), ExecutorServiceIllegalStateException, 100601);
}

/** １件Threadをstartすると正常にTaskが動作する */
TEST_F(QueuedThreadPoolIntegrationTest, start_TaskExecuted_WhenSingleThread)
{
    // Given: QueuedThreadPoolを生成
    QueuedThreadPool queuedThreadPool(m_pBoundBlockingQueue);

    // When: １件Threadをstartさせてwait
    Task::Ptr pTestTask = new TestTask(this);
    queuedThreadPool.start(pTestTask);

    // Then: Taskが呼び出されている
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    EXPECT_EQ(0, m_pushCount.value());
}

/** Poco::ThreadPoolのmaxCapacityを超えないTaskをまとめてstartした場合、全てのrunが呼び出される */
TEST_F(QueuedThreadPoolIntegrationTest, start_AllTasksExecuted_WhenLessThanMaxCapacityOfThreadPool)
{
    // Given: QueuedThreadPoolを生成(ThreadPoolのmaxCapacityは16に設定)
    QueuedThreadPool queuedThreadPool(2, 16, m_pBoundBlockingQueue);

    // When: 10件Threadをstartさせてwait
    m_threadCount = 10;
    for (int i = 0; i < m_threadCount; i++) {
        Task::Ptr pTestTask = new TestTask(this, 500);
        queuedThreadPool.start(pTestTask);
    }

    // Then: 全てのTaskが呼び出されている
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    EXPECT_EQ(0, m_pushCount);
}

/** Poco::ThreadPoolのmaxCapacityを超えるTaskをまとめてstartした場合、全てのrunが呼び出される */
TEST_F(QueuedThreadPoolIntegrationTest, start_AllTasksExecuted_WhenGreaterThanMaxCapacityOfThreadPool)
{
    // Given: QueuedThreadPoolを生成(ThreadPoolのmaxCapacityは5に設定)
    //    QueuedThreadPool queuedThreadPool(2, 5);
    QueuedThreadPool queuedThreadPool(2, 5, m_pBoundBlockingQueue);

    // When: 10件Threadをstartさせてwait
    m_threadCount = 10;
    for (int i = 0; i < m_threadCount; i++) {
        Task::Ptr pTestTask = new TestTask(this, 500);
        queuedThreadPool.start(pTestTask);
    }

    // Then: 全てのTaskが呼び出されている
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    EXPECT_EQ(5, m_pushCount);
}

/** Poco::QueuedThreadPoolのmaxQueueSizeを超えるTaskをまとめてstartした場合、ExceptionがThrowされる */
TEST_F(QueuedThreadPoolIntegrationTest, start_ThrowException_WhenGreaterThanMaxQueueSizeOfQueuedThreadPool)
{
    // Given: QueuedThreadPoolを生成(ThreadPoolのmaxCapacityは16に設定)
    QueuedThreadPool queuedThreadPool(2, 16, m_pBoundBlockingQueue);

    // When: 上限までThreadをstartさせてwait
    m_threadCount = 16 + BoundBlockingQueue::DefaultMaxQueueSize;
    // Poco::ThreadPoolから呼び出されるTaskはEventでwaitする
    for (int i = 0; i < 16; i++) {
        Task::Ptr pWaitTask = new WaitTask(this, 5000);
        queuedThreadPool.start(pWaitTask);
    }
    // Poco::ThreadPoolのmaxCapacityを超えた分はBlockingQueueに保持されるのでsleepを短くする
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        Task::Ptr pTestTask = new TestTask(this);
        queuedThreadPool.start(pTestTask);
    }

    // Then: QueuedThreadPoolのmaxQueueSize分start分Taskを溜めて、startを呼び出すと
    //       ExecutorServiceTooManyRequestsExceptionがThrowされる
    Task::Ptr pTestTask = new TestTask(this);
    EASYHTTPCPP_EXPECT_THROW(queuedThreadPool.start(pTestTask), ExecutorServiceTooManyRequestsException, 100603);

    m_pWorkerThreadBlocker->set();
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    EXPECT_EQ((unsigned int) BoundBlockingQueue::DefaultMaxQueueSize, m_pushCount);
}

/** Poco::QueuedThreadPoolのコンストラクタでUnboundBlockingQueueを指定した場合、何件TaskをstartしてもExceptionがThrowされない */
TEST_F(QueuedThreadPoolIntegrationTest, start_NotThrowException_WhenUsedUnboundBlockingQueue)
{
    // Given: UnboundBlockingQueueを指定してQueuedThreadPoolを生成(ThreadPoolのmaxCapacityは16に設定)
    UnboundBlockingQueue::Ptr pBlockingQueue = new UnboundBlockingQueue();
    QueuedThreadPool queuedThreadPool(2, 16, pBlockingQueue);

    // When: BoundBlockingQueueの上限値を超える分のThreadをstartさせてwait
    m_threadCount = 16 + BoundBlockingQueue::DefaultMaxQueueSize + 1;
    // Poco::ThreadPoolから呼び出されるTaskはeventでwaitする
    for (int i = 0; i < 16; i++) {
        Task::Ptr pWaitTask = new WaitTask(this, 5000);
        queuedThreadPool.start(pWaitTask);
    }
    // Poco::ThreadPoolのmaxCapacityを超えた分はBlockingQueueに保持されるのでsleepを短くする
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        Task::Ptr pTestTask = new TestTask(this);
        queuedThreadPool.start(pTestTask);
    }

    // Then: QueuedThreadPoolのmaxQueueSize分start分Taskを溜めて、startを呼び出すと
    //       ExecutorServiceTooManyRequestsExceptionがThrowされずに全てのTaskが呼び出される
    Task::Ptr pTestTask = new TestTask(this);
    EXPECT_NO_THROW(queuedThreadPool.start(pTestTask));

    m_pWorkerThreadBlocker->set();
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
}

/** ランタイムが異なるTaskをマルチスレッドで走らせた場合、全てのTaskが正常に呼び出される */
TEST_F(QueuedThreadPoolIntegrationTest, start_AllTasksExecuted_WhenRandomTimeByMultiThread)
{
    // Given: QueuedThreadPoolを生成(ThreadPoolのmaxCapacityは16に設定)
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(2, 16);

    // When: マルチスレッドでTaskをstartする
    m_threadCount = 100;
    Poco::Random random;
    std::queue<Poco::SharedPtr<Poco::Thread> > threadList;
    for (int i = 0; i < m_threadCount; i++) {
        QueuedThreadPoolStartTask* pStartTask =
                new QueuedThreadPoolStartTask(this, pQueuedThreadPool, random.next(10));
        Poco::SharedPtr<Poco::Thread> pTh = new Poco::Thread();
        pTh->start(*pStartTask);
        threadList.push(pTh);
    }

    // Then: 全てのTaskが呼び出される
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    while (!threadList.empty()) {
        Poco::SharedPtr<Poco::Thread> pTh = threadList.front();
        pTh->join();
        threadList.pop();
    }
}

/** マルチスレッドで同時にTaskを走らせた場合、全てのTaskが正常に呼び出される */
TEST_F(QueuedThreadPoolIntegrationTest, start_AllTasksExecuted_WhenSameTimeByMultiThread)
{
    // Given: QueuedThreadPoolを生成(ThreadPoolのmaxCapacityは16に設定)
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(2, 16);

    // When: マルチスレッドでTaskをstartする
    m_threadCount = 100;
    std::queue<Poco::SharedPtr<Poco::Thread> > threadList;
    for (int i = 0; i < m_threadCount; i++) {
        QueuedThreadPoolStartTask* pStartTask =
                new QueuedThreadPoolStartTask(this, pQueuedThreadPool, true);
        Poco::SharedPtr<Poco::Thread> pTh = new Poco::Thread();
        pTh->start(*pStartTask);
        threadList.push(pTh);
    }
    // waitを解除して同時にQueuedThreadPool::startを呼び出す
    m_pWorkerThreadBlocker->set();

    // Then: 全てのTaskが呼び出される
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
    while (!threadList.empty()) {
        Poco::SharedPtr<Poco::Thread> pTh = threadList.front();
        pTh->join();
        threadList.pop();
    }
}

/** shutdownを呼び出しても、キューイングされたTaskは消されない */
TEST_F(QueuedThreadPoolIntegrationTest, shutdown_NotClearTasks)
{
    // Given: startを呼び出してQueueを溜めておく
    TestBoundBlockingQueue::Ptr pWorkerQueue = new TestBoundBlockingQueue(this);
    QueuedThreadPool queuedThreadPool(2, 16, pWorkerQueue);
    m_threadCount = 16 + 3;
    for (unsigned int i = 0; i < 16; i++) {
        Task::Ptr pWaitTask = new WaitTask(this, 1000);
        queuedThreadPool.start(pWaitTask);
    }
    for (unsigned int i = 0; i < 3; i++) {
        Task::Ptr pTestTask = new TestTask(this, 1);
        queuedThreadPool.start(pTestTask);
    }
    EXPECT_FALSE(pWorkerQueue->isEmpty());
    EXPECT_EQ(3, pWorkerQueue->getQueueSize());

    // When: shutdownを呼び出す
    queuedThreadPool.shutdown();

    // Then: Queueが削除されていない
    EXPECT_FALSE(pWorkerQueue->isEmpty());
    EXPECT_EQ(3, pWorkerQueue->getQueueSize());

    m_pWorkerThreadBlocker->set();
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
}

/** shutdownを繰り返し呼び出しても、正常に呼び出せる */
TEST_F(QueuedThreadPoolIntegrationTest, shutdown_Succeeds_WhenCalledRepeatly)
{
    // Given: startを呼び出してQueueを溜めた後、一度shutdownを呼び出しておく
    TestBoundBlockingQueue::Ptr pWorkerQueue = new TestBoundBlockingQueue(this);
    QueuedThreadPool queuedThreadPool(2, 16, pWorkerQueue);
    m_threadCount = 16 + 3;
    for (unsigned int i = 0; i < 16; i++) {
        Task::Ptr pWaitTask = new WaitTask(this, 1000);
        queuedThreadPool.start(pWaitTask);
    }
    for (unsigned int i = 0; i < 3; i++) {
        Task::Ptr pTestTask = new TestTask(this, 1);
        queuedThreadPool.start(pTestTask);
    }
    EXPECT_FALSE(pWorkerQueue->isEmpty());
    EXPECT_EQ(3, pWorkerQueue->getQueueSize());

    // shutdownを呼び出す
    queuedThreadPool.shutdown();
    // Queueは削除されていない
    EXPECT_FALSE(pWorkerQueue->isEmpty());
    EXPECT_EQ(3, pWorkerQueue->getQueueSize());

    // When: 再度shutdownを呼び出す
    queuedThreadPool.shutdown();

    // Then: Queueは削除されていない
    EXPECT_FALSE(pWorkerQueue->isEmpty());
    EXPECT_EQ(3, pWorkerQueue->getQueueSize());

    m_pWorkerThreadBlocker->set();
    EXPECT_NO_THROW(m_pMainThreadBlocker->wait(10000));
}

TEST_F(QueuedThreadPoolIntegrationTest, shutdownAndJoinAll_WaitsForAllTasksToComplete)
{
    // Given: some tasks are started
    QueuedThreadPool queuedThreadPool;
    TestFutureTask::Ptr pTestTask1 = new TestFutureTask(1000);
    queuedThreadPool.start(pTestTask1);
    TestFutureTask::Ptr pTestTask2 = new TestFutureTask(500);
    queuedThreadPool.start(pTestTask2);
    TestFutureTask::Ptr pTestTask3 = new TestFutureTask(100);
    queuedThreadPool.start(pTestTask3);

    // When: call shutdownAndJoinAll()
    queuedThreadPool.shutdownAndJoinAll();

    // Then: all tasks are completed
    EXPECT_TRUE(pTestTask1->isDone());
    EXPECT_TRUE(pTestTask2->isDone());
    EXPECT_TRUE(pTestTask3->isDone());
}

TEST_F(QueuedThreadPoolIntegrationTest, shutdownAndJoinAll_WaitsForAllTasksToComplete_WhenCalledAfterShutdown)
{
    // Given: some tasks are started and shutdown() is called
    QueuedThreadPool queuedThreadPool;
    TestFutureTask::Ptr pTestTask1 = new TestFutureTask(1000);
    queuedThreadPool.start(pTestTask1);
    TestFutureTask::Ptr pTestTask2 = new TestFutureTask(500);
    queuedThreadPool.start(pTestTask2);
    TestFutureTask::Ptr pTestTask3 = new TestFutureTask(100);
    queuedThreadPool.start(pTestTask3);

    queuedThreadPool.shutdown();

    // When: call shutdownAndJoinAll()
    queuedThreadPool.shutdownAndJoinAll();

    // Then: all tasks are completed
    EXPECT_TRUE(pTestTask1->isDone());
    EXPECT_TRUE(pTestTask2->isDone());
    EXPECT_TRUE(pTestTask3->isDone());
}

TEST_F(QueuedThreadPoolIntegrationTest, shutdownAndJoinAll_Succeeds_WhenCalledRepeatly)
{
    // Given: start a task then call shutdownAndJoinAll()
    QueuedThreadPool queuedThreadPool;
    TestFutureTask::Ptr pTestTask1 = new TestFutureTask(100);
    queuedThreadPool.start(pTestTask1);
    queuedThreadPool.shutdownAndJoinAll();
    EXPECT_TRUE(pTestTask1->isDone());

    // When: call shutdownAndJoinAll() again
    // Then: returns normally
    queuedThreadPool.shutdownAndJoinAll();
}

TEST_F(QueuedThreadPoolIntegrationTest, getNextTask_ReturnsNull_WhenCalledAfterShutdownAndJoinAll)
{
    // Given: some tasks are started and shutdownAndJoinAll() is called
    QueuedThreadPool queuedThreadPool;
    TestFutureTask::Ptr pTestTask1 = new TestFutureTask(1000);
    queuedThreadPool.start(pTestTask1);
    TestFutureTask::Ptr pTestTask2 = new TestFutureTask(500);
    queuedThreadPool.start(pTestTask2);
    TestFutureTask::Ptr pTestTask3 = new TestFutureTask(100);
    queuedThreadPool.start(pTestTask3);

    queuedThreadPool.shutdownAndJoinAll();

    // When: call getNextTask()
    // Then: NULL is returned
    EXPECT_TRUE(queuedThreadPool.getNextTask().isNull());

    // all tasks are completed
    EXPECT_TRUE(pTestTask1->isDone());
    EXPECT_TRUE(pTestTask2->isDone());
    EXPECT_TRUE(pTestTask3->isDone());
}

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
