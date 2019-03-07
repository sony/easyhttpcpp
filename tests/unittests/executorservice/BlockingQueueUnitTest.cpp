/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/executorservice/BoundBlockingQueue.h"
#include "easyhttpcpp/executorservice/Task.h"

namespace easyhttpcpp {
namespace executorservice {
namespace test {

using easyhttpcpp::common::RefCountedRunnable;

class BlockingQueueUnitTest : public testing::TestWithParam<bool> {
protected:

    BlockingQueue::Ptr createBlockingQueue(bool bound)
    {
        if (bound) {
            return new BoundBlockingQueue();
        } else {
            return new UnboundBlockingQueue();
        }
    }

    unsigned int getBlockingQueueSize(BlockingQueue::Ptr pBlockingQueue)
    {
        unsigned int queueSize = 0;
        while (!pBlockingQueue->isEmpty()) {
            pBlockingQueue->pop();
            queueSize++;
        }
        return queueSize;
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
};
// このフィクスチャでTEST_Pを使っているテストは、パラメータがfalse/trueの順で２回テストが呼び出される
// このパラメータでclassを切り替えてUnboundBlockingQueue/BoundBlockingQueueのテストを行っている
//   false : UnboundBlockingQueue
//   true  : BoundBlockingQueue
INSTANTIATE_TEST_CASE_P(BoolParam, BlockingQueueUnitTest, testing::Bool());

// pushを一度だけ呼び出すとtrueが返る
TEST_P(BlockingQueueUnitTest, push_ReturnsTrue_WhenCallsAtOnce)
{
    // Given: BlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: pushを呼び出す
    // Then: trueが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    EXPECT_EQ(1, getBlockingQueueSize(pBlockingQueue));
}

// BoundBlockingQueueの場合、MaxQueueSizeをセットせずにpushをMaxQueueSizeのデフォルト値を超えて呼び出すとfalseが返る
TEST_F(BlockingQueueUnitTest, pushInBoundBlockingQueue_ReturnsFalse_WhenRepeatedCallsGreaterThanDefaultMaxQueueSize)
{
    // Given: MaxQueueSizeを指定せずにBoundBlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(true);

    // When: MaxQueueSizeのデフォルト値までpushする
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    }

    // Then: MaxQueueSizeまでpushした後、pushするとfalseが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_FALSE(pBlockingQueue->push(pTestTask));
    EXPECT_EQ(BoundBlockingQueue::DefaultMaxQueueSize, getBlockingQueueSize(pBlockingQueue));
}

// UnboundBlockingQueueの場合、pushをMaxQueueSizeのデフォルト値を超えて呼び出してもtrueが返る
TEST_F(BlockingQueueUnitTest, pushInUnboundBlockingQueue_ReturnsTrue_WhenRepeatedCallsGreaterThanDefaultMaxQueueSize)
{
    // Given: MaxQueueSizeを指定せずにUnboundBlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(false);

    // When: MaxQueueSizeのデフォルト値までpushする
    for (unsigned int i = 0; i < BoundBlockingQueue::DefaultMaxQueueSize; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    }

    // Then: MaxQueueSizeまでpushした後、pushしてもtrueが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    EXPECT_EQ(BoundBlockingQueue::DefaultMaxQueueSize + 1, getBlockingQueueSize(pBlockingQueue));
}

// MaxQueueSizeをセットしてpushをセットした値を超えて呼び出すとfalseが返る(BoundBlockingQueueのみ)
TEST_F(BlockingQueueUnitTest,
        pushInBoundBlockingQueue_ReturnsFalse_WhenRepeatedCallsGreaterThanMaxValueWithSetMaxQueueSize)
{
    // Given: MaxQueueSizeを指定してにBoundBlockingQueueを生成
    unsigned int maxQueueSize = 10;
    BlockingQueue::Ptr pBlockingQueue = new BoundBlockingQueue(maxQueueSize);

    // When: MaxQueueSizeのデフォルト値までpushする
    for (unsigned int i = 0; i < maxQueueSize; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    }

    // Then: MaxQueueSizeまでpushした後、pushするとfalseが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_FALSE(pBlockingQueue->push(pTestTask));
    EXPECT_EQ(maxQueueSize, getBlockingQueueSize(pBlockingQueue));
}

// pushの後にpopを呼び出すとTaskが取得できる
TEST_P(BlockingQueueUnitTest, pop_ReturnsThePushedTask)
{
    // Given: Taskをpushしておく
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_TRUE(pBlockingQueue->push(pTestTask));

    // When: popを呼び出す
    RefCountedRunnable::Ptr pPopTask = pBlockingQueue->pop();

    // Then: pushしたTaskが取得できる
    EXPECT_NE((RefCountedRunnable::Ptr) NULL, pPopTask);
    EXPECT_EQ(pTestTask, pPopTask);
}

// pushを呼ばずにpopを呼び出すとNULLが返る
TEST_P(BlockingQueueUnitTest, pop_ReturnsNull_WhenNotCalledPush)
{
    // Given: Taskをpushしない
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: popを呼び出す
    // Then: NULLが返る
    EXPECT_EQ((RefCountedRunnable::Ptr) NULL, pBlockingQueue->pop());
}

// pushより多くpopを呼び出すとNULLが返る
TEST_P(BlockingQueueUnitTest, pop_ReturnsNull_WhenCalledMoreThanPush)
{
    // Given: Taskをpushしておく
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());
    for (int i = 0; i < 3; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    }

    // When: pushより多くpopを呼び出す
    // Then: NULLが返る
    for (int i = 0; i < 3; i++) {
        EXPECT_NE((RefCountedRunnable::Ptr) NULL, pBlockingQueue->pop());
    }
    EXPECT_EQ((RefCountedRunnable::Ptr) NULL, pBlockingQueue->pop());
}

// pushを呼ばずにisEmptyを呼び出すとtrueが返る
TEST_P(BlockingQueueUnitTest, isEmpty_ReturnsTrue_WhenNotCalledPush)
{
    // Given: BlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: TaskをpushせずにisEmptyを呼び出す
    // Then: trueが返る
    EXPECT_TRUE(pBlockingQueue->isEmpty());
}

// pushを呼んだ後にisEmptyを呼び出すとfalseが返る
TEST_P(BlockingQueueUnitTest, isEmpty_ReturnsFalse_WhenCalledPush)
{
    // Given: BlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: Taskをpushした後にisEmptyを呼び出す
    // Then: falseが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    EXPECT_FALSE(pBlockingQueue->isEmpty());
}

// push,popを呼んだ後にisEmptyを呼び出すとtrueが返る
TEST_P(BlockingQueueUnitTest, isEmpty_ReturnsTrue_WhenCalledPushAndPop)
{
    // Given: BlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: push,popを呼んだ後にisEmptyを呼び出す
    // Then: trueが返る
    RefCountedRunnable::Ptr pTestTask = new TestTask();
    EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    EXPECT_NE((RefCountedRunnable::Ptr) NULL, pBlockingQueue->pop());
    EXPECT_TRUE(pBlockingQueue->isEmpty());
}

// キューが空の時でも、clearを呼び出すことができる
TEST_P(BlockingQueueUnitTest, clear_Succeeds_WhenQueueIsEmpty)
{
    // Given: BlockingQueueを生成
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());

    // When: キューが空の時にclearを呼び出す
    // Then: clearは正常を正常に呼び出せてQueueは空のまま
    pBlockingQueue->clear();
    EXPECT_TRUE(pBlockingQueue->isEmpty());
}

// pushを呼んだ後、clearを呼び出すとキューイングしたTaskが全て消される
TEST_P(BlockingQueueUnitTest, clear_RemoveAllTasks_WhenQueueHasSomeTasks)
{
    // Given: BlockingQueueを生成し、pushを呼んでTaskを溜めておく
    BlockingQueue::Ptr pBlockingQueue = createBlockingQueue(GetParam());
    for (int i = 0; i < 3; i++) {
        RefCountedRunnable::Ptr pTestTask = new TestTask();
        EXPECT_TRUE(pBlockingQueue->push(pTestTask));
    }

    // When: clearを呼び出す
    pBlockingQueue->clear();

    // Then: キューイングされたTaskが消えている
    EXPECT_TRUE(pBlockingQueue->isEmpty());
}

} /* namespace test */
} /* namespace executorservice */
} /* namespace easyhttpcpp */
