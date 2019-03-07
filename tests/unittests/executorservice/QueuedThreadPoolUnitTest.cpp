/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/executorservice/BoundBlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"
#include "easyhttpcpp/executorservice/UnboundBlockingQueue.h"
#include "easyhttpcpp/executorservice/Task.h"
#include "EasyHttpCppAssertions.h"

using ::testing::Return;
using easyhttpcpp::common::RefCountedRunnable;

namespace easyhttpcpp {
namespace executorservice {
namespace test {

class QueuedThreadPoolUnitTest : public testing::Test {
protected:

    static Poco::Event s_taskWaitEvent;

    void SetUp()
    {
        s_taskWaitEvent.reset();
    }

    class TestTask : public Task {
    public:
        TestTask()
        {
        }

        void runTask()
        {
        }
    };

    class WaitTask : public Task {
    public:

        WaitTask()
        {
        }

        void runTask()
        {
            s_taskWaitEvent.wait();
        }
    };

    class MockBoundBlockingQueue : public BoundBlockingQueue {
    public:
        MOCK_METHOD1(push, bool(RefCountedRunnable::Ptr pTask));

        MOCK_METHOD0(pop, RefCountedRunnable::Ptr());

        MOCK_METHOD0(isEmpty, bool());

        MOCK_METHOD0(clear, void());
    };

    class MockUnboundBlockingQueue : public UnboundBlockingQueue {
    public:
        MOCK_METHOD1(push, bool(RefCountedRunnable::Ptr pTask));

        MOCK_METHOD0(pop, RefCountedRunnable::Ptr());

        MOCK_METHOD0(isEmpty, bool());

        MOCK_METHOD0(clear, void());
    };
};

Poco::Event QueuedThreadPoolUnitTest::s_taskWaitEvent(false);

// パラメータを指定せずにコンストラクタを呼び出すとデフォルト設定でQueuedThreadPoolが生成される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_UsedDefaultSetting_WhenNoParameter)
{
    // Given:

    // When: パラメータを指定せずにQueuedThreadPoolを生成
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool();

    // Then: デフォルトの設定でQueuedThreadPoolが生成されている
    // corePoolSize
    EXPECT_EQ(QueuedThreadPool::DefaultCorePoolSize, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize
    EXPECT_EQ(QueuedThreadPool::DefaultMaximumPoolSize, pQueuedThreadPool->getMaximumPoolSize());

    // 未指定なのでBoundBlockingQueueが使われる
    // Queueが上限を超えるとThrowされる
    for (unsigned int i = 0; i < QueuedThreadPool::DefaultMaximumPoolSize; i++) {
        RefCountedRunnable::Ptr pWaitTask = new WaitTask();
        EXPECT_NO_THROW(pQueuedThreadPool->start(pWaitTask));
    }
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_NO_THROW(pQueuedThreadPool->start(pTestTask));
    }
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EASYHTTPCPP_EXPECT_THROW(pQueuedThreadPool->start(pTestTask), ExecutorServiceTooManyRequestsException, 100603);
    s_taskWaitEvent.set();
}

// corePoolSize, maximumPoolSizeを指定してコンストラクタを呼び出すと指定した値が反映される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_UsedParameter_WhenSetPoolSize)
{
    // Given:

    // When: corePoolSize, maximumPoolSizeを指定してQueuedThreadPoolを生成
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(5, 10);

    // Then: 指定した値が反映されている
    // corePoolSize
    EXPECT_EQ(5, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize
    EXPECT_EQ(10, pQueuedThreadPool->getMaximumPoolSize());

    // 未指定なのでBoundBlockingQueueが使われる
    // Queueが上限を超えるとThrowされる
    for (int i = 0; i < 10; i++) {
        RefCountedRunnable::Ptr pWaitTask = new WaitTask();
        EXPECT_NO_THROW(pQueuedThreadPool->start(pWaitTask));
    }
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_NO_THROW(pQueuedThreadPool->start(pTestTask));
    }
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EASYHTTPCPP_EXPECT_THROW(pQueuedThreadPool->start(pTestTask), ExecutorServiceTooManyRequestsException, 100603);
    s_taskWaitEvent.set();
}

// BoundBlockingQueueを指定してコンストラクタを呼び出すと指定したBlockingQueueが反映される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_UsedParameter_WhenSetBoundBlockingQueue)
{
    // Given:

    // When: BoundBlockingQueueのMockを指定してQueuedThreadPoolを生成
    MockBoundBlockingQueue* pMockBlockingQueue = new MockBoundBlockingQueue();
    EXPECT_CALL(*pMockBlockingQueue, pop()).Times(1).WillOnce(Return(RefCountedRunnable::Ptr()));
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(pMockBlockingQueue);

    // Then: 指定したBlockingQueueが反映されている
    // corePoolSize(指定していないのでデフォルト値)
    EXPECT_EQ(QueuedThreadPool::DefaultCorePoolSize, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize(指定していないのでデフォルト値)
    EXPECT_EQ(QueuedThreadPool::DefaultMaximumPoolSize, pQueuedThreadPool->getMaximumPoolSize());

    // 指定したBlockingQueueのmethodが呼び出されることを確認
    EXPECT_TRUE(pQueuedThreadPool->getNextTask().isNull());
}

// UnboundBlockingQueueを指定してコンストラクタを呼び出すと指定したBlockingQueueが反映される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_UsedParameter_WhenSetUnboundBlockingQueue)
{
    // Given:

    // When: UnboundBlockingQueueのMockを指定してQueuedThreadPoolを生成
    MockUnboundBlockingQueue* pMockBlockingQueue = new MockUnboundBlockingQueue();
    EXPECT_CALL(*pMockBlockingQueue, pop()).Times(1).WillOnce(Return(RefCountedRunnable::Ptr()));
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(pMockBlockingQueue);

    // Then: 指定したBlockingQueueが反映されている
    // corePoolSize(指定していないのでデフォルト値)
    EXPECT_EQ(QueuedThreadPool::DefaultCorePoolSize, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize(指定していないのでデフォルト値)
    EXPECT_EQ(QueuedThreadPool::DefaultMaximumPoolSize, pQueuedThreadPool->getMaximumPoolSize());

    // 指定したBlockingQueueのmethodが呼び出されることを確認
    EXPECT_TRUE(pQueuedThreadPool->getNextTask().isNull());
}

// corePoolSize, maximumPoolSize, BoundBlockingQueueを指定してコンストラクタを呼び出すと指定したパラメータが反映される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPoolOfFullParameter_UsedParameter_WhenSetPoolSizeAndBoundBlockingQueue)
{
    // Given:

    // When: corePoolSize, maximumPoolSize, BoundBlockingQueueのMockを指定してQueuedThreadPoolを生成
    MockBoundBlockingQueue* pMockBlockingQueue = new MockBoundBlockingQueue();
    EXPECT_CALL(*pMockBlockingQueue, pop()).Times(1).WillOnce(Return(RefCountedRunnable::Ptr()));
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(5, 10, pMockBlockingQueue);

    // Then: 指定したcorePoolSize, maximumPoolSize, BlockingQueueが反映されている
    // corePoolSize
    EXPECT_EQ(5, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize
    EXPECT_EQ(10, pQueuedThreadPool->getMaximumPoolSize());

    // 指定したBlockingQueueのmethodが呼び出されることを確認
    EXPECT_TRUE(pQueuedThreadPool->getNextTask().isNull());
}

// corePoolSize, maximumPoolSize, UnboundBlockingQueueを指定してコンストラクタを呼び出すと指定したパラメータが反映される
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPoolOfFullParameter_UsedParameter_WhenSetPoolSizeAndUnboundBlockingQueue)
{
    // Given:

    // When: corePoolSize, maximumPoolSize, UnboundBlockingQueueのMockを指定してQueuedThreadPoolを生成
    MockUnboundBlockingQueue* pMockBlockingQueue = new MockUnboundBlockingQueue();
    EXPECT_CALL(*pMockBlockingQueue, pop()).Times(1).WillOnce(Return(RefCountedRunnable::Ptr()));
    QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(5, 10, pMockBlockingQueue);

    // Then: 指定したcorePoolSize, maximumPoolSize, BlockingQueueが反映されている
    // corePoolSize
    EXPECT_EQ(5, pQueuedThreadPool->getCorePoolSize());

    // maximumPoolSize
    EXPECT_EQ(10, pQueuedThreadPool->getMaximumPoolSize());

    // 指定したBlockingQueueのmethodが呼び出されることを確認
    EXPECT_TRUE(pQueuedThreadPool->getNextTask().isNull());
}

// corePoolSizeに0を指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_ThrowException_WhenCorePoolSizeIsZero)
{
    // Given:

    // When: corePoolSizeに0を指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(0, 10),
            ExecutorServiceIllegalArgumentException, 100600);
}

// maximumPoolSizeより大きいcorePoolSizeを指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_ThrowException_WhenCorePoolSizeIsGreaterThanMaximumPoolSize)
{
    // Given:

    // When: maximumPoolSizeより大きいcorePoolSizeを指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(10, 5),
            ExecutorServiceIllegalArgumentException, 100600);
}

// BlockingQueueにNULLを指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPool_ThrowException_WhenBlockingQueueIsNull)
{
    // Given:

    // When: BlockingQueueにNULLを指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(NULL),
            ExecutorServiceIllegalArgumentException, 100600);
}

// corePoolSizeに0を指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPoolOfFullParameter_ThrowException_WhenCorePoolSizeIsZero)
{
    // Given:

    // When: corePoolSizeに0を指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    UnboundBlockingQueue::Ptr pBlockingQueue = new UnboundBlockingQueue();
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(0, 10, pBlockingQueue),
            ExecutorServiceIllegalArgumentException, 100600);
}

// maximumPoolSizeより大きいcorePoolSizeを指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest,
        queuedThreadPoolOfFullParameter_ThrowException_WhenCorePoolSizeIsGreaterThanMaximumPoolSize)
{
    // Given:

    // When: maximumPoolSizeより大きいcorePoolSizeを指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    UnboundBlockingQueue::Ptr pBlockingQueue = new UnboundBlockingQueue();
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(10, 5, pBlockingQueue),
            ExecutorServiceIllegalArgumentException, 100600);
}

// BlockingQueueにNULLを指定してコンストラクタを呼び出すとExceptionがthrowされる
TEST_F(QueuedThreadPoolUnitTest, queuedThreadPoolOfFullParameter_ThrowException_WhenBlockingQueueIsNull)
{
    // Given:

    // When: BlockingQueueにNULLを指定してQueuedThreadPoolを生成
    // Then: ExecutorServiceIllegalArgumentExceptionがthrowされる
    EASYHTTPCPP_EXPECT_THROW(QueuedThreadPool::Ptr pQueuedThreadPool = new QueuedThreadPool(2, 16, NULL),
            ExecutorServiceIllegalArgumentException, 100600);
}

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
