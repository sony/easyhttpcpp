/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/ThreadPool.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/executorservice/BoundBlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"

#include "RunnableWorker.h"

namespace easyhttpcpp {
namespace executorservice {

using easyhttpcpp::common::RefCountedRunnable;

const unsigned int QueuedThreadPool::DefaultCorePoolSize = 2;
const unsigned int QueuedThreadPool::DefaultMaximumPoolSize = 5;

static const std::string Tag = "QueuedThreadPool";
static const unsigned int IdleTime = 10;

QueuedThreadPool::QueuedThreadPool()
{
    initialize(DefaultCorePoolSize, DefaultMaximumPoolSize, static_cast<BlockingQueue::Ptr>(new BoundBlockingQueue()));
}

QueuedThreadPool::QueuedThreadPool(unsigned int corePoolSize, unsigned int maximumPoolSize)
{
    initialize(corePoolSize, maximumPoolSize, static_cast<BlockingQueue::Ptr>(new BoundBlockingQueue()));
}

QueuedThreadPool::QueuedThreadPool(unsigned int corePoolSize, unsigned int maximumPoolSize,
        BlockingQueue::Ptr pWorkerQueue)
{
    initialize(corePoolSize, maximumPoolSize, pWorkerQueue);
}

QueuedThreadPool::QueuedThreadPool(BlockingQueue::Ptr pWorkerQueue)
{
    initialize(DefaultCorePoolSize, DefaultMaximumPoolSize, pWorkerQueue);
}

QueuedThreadPool::~QueuedThreadPool()
{
    // Waits for all threads to complete.
    //
    // Note that this will not actually join() the underlying
    // thread, but rather wait for the thread's runnables
    // to finish.
    m_pThreadPool->joinAll();
    // Stops all running threads and waits for their completion.
    m_pThreadPool->stopAll();
}

void QueuedThreadPool::start(RefCountedRunnable::Ptr pTask)
{
    // throw exception when already called shutdown
    if (isTerminated()) {
        EASYHTTPCPP_LOG_D(Tag, "shutdown is already called");
        throw ExecutorServiceIllegalStateException("Can not start task, QueuedThreadPool is already terminated");
    }

    if (!pTask) {
        EASYHTTPCPP_LOG_D(Tag, "Task is NULL");
        throw ExecutorServiceIllegalArgumentException("Can not start task, task is not allow NULL");
    }

    // create RunnableWorker
    RunnableWorker* pWorker = new RunnableWorker(pTask, this);

    // start ThreadPool of POCO
    Poco::FastMutex::ScopedLock lockQueue(m_queueMutex);
    
    pTask->duplicate(); // run will release
    while (true) {
        try {
            m_pThreadPool->start(static_cast<Poco::Runnable&> (*pWorker));

            m_activeWorkerCount++;
        } catch (const Poco::NoThreadAvailableException&) {
            if (m_activeWorkerCount == 0) {
                // Retry because Thread can be used soon
                Poco::Thread::sleep(1);
                continue;
            }
            delete pWorker;
            // push Runnable to BlockingQueue when NoThreadAvailableException is thrown
            EASYHTTPCPP_LOG_D(Tag, "Catch NoThreadAvailableException when call Poco::ThreadPool::start");

            if (!m_pBlockingQueue->push(pTask)) {
                EASYHTTPCPP_LOG_D(Tag, "number of task is greater than max queue size");
                
                pTask->release(); // task will no longer run; release here
                
                throw ExecutorServiceTooManyRequestsException(
                        "Can not run task, number of task is greater than max queue size");
            }
        } catch (const Poco::Exception& e) {
            delete pWorker;
            pTask->release(); // task will no longer run; release here
            
            std::string message = "Can not start task, system error has occurred";
            EASYHTTPCPP_LOG_D(Tag, "Catch unknown Exception when call Poco::ThreadPool::start : %s", e.message().c_str());
            throw ExecutorServiceExecutionException(message, e);
        }
        break;
    }
}

// Return next task and decrement m_activeWorkerCount if BlockingQueue is empty
RefCountedRunnable::Ptr QueuedThreadPool::getNextTask()
{
    Poco::FastMutex::ScopedLock lockQueue(m_queueMutex);
    RefCountedRunnable::Ptr pTask = m_pBlockingQueue->pop();
    if (!pTask) {
        m_activeWorkerCount--;
    }
    return pTask;
}

void QueuedThreadPool::shutdown()
{
    Poco::FastMutex::ScopedLock lockTerminated(m_terminatedMutex);
    m_terminated = true;
    EASYHTTPCPP_LOG_D(Tag, "shutdown was executed");
}

void QueuedThreadPool::shutdownAndJoinAll()
{
    {
        // shutdown
        Poco::FastMutex::ScopedLock lockTerminated(m_terminatedMutex);
        m_terminated = true;
    }

    // Waits for all threads to complete.
    //
    // Note that this will not actually join() the underlying
    // thread, but rather wait for the thread's runnables
    // to finish.
    m_pThreadPool->joinAll();

    // Stops all running threads and waits for their completion.
    m_pThreadPool->stopAll();

    EASYHTTPCPP_LOG_D(Tag, "shutdownAndJoinAll was executed");
}

unsigned int QueuedThreadPool::getCorePoolSize() const
{
    return static_cast<unsigned int>(m_pThreadPool->allocated());
}

unsigned int QueuedThreadPool::getMaximumPoolSize() const
{
    return static_cast<unsigned int>(m_pThreadPool->capacity());
}

void QueuedThreadPool::initialize(unsigned int corePoolSize, unsigned int maximumPoolSize,
        BlockingQueue::Ptr pWorkerQueue)
{
    if (corePoolSize == 0 || maximumPoolSize < corePoolSize) {
        EASYHTTPCPP_LOG_D(Tag, "corePoolSize = %u, maximumPoolSize = %u", corePoolSize, maximumPoolSize);
        throw ExecutorServiceIllegalArgumentException("Can not generate QueuedThreadPool, "
                "corePoolSize is not allow 0 and corePoolSize should be smaller than maximumPoolSize");
    }

    if (!pWorkerQueue) {
        EASYHTTPCPP_LOG_D(Tag, "BlockingQueue is NULL");
        throw ExecutorServiceIllegalArgumentException(
                "Can not generate QueuedThreadPool, BlockingQueue is not allow NULL");
    }

    m_pThreadPool = new Poco::ThreadPool(static_cast<int>(corePoolSize), static_cast<int>(maximumPoolSize), IdleTime);
    m_pBlockingQueue = pWorkerQueue;
    m_activeWorkerCount = 0;
    m_terminated = false;
}

bool QueuedThreadPool::isTerminated()
{
    Poco::FastMutex::ScopedLock lockTerminated(m_terminatedMutex);
    return m_terminated;
}

} /* namespace executorservice */
} /* namespace easyhttpcpp */
