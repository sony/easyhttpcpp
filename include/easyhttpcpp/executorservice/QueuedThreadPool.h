/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_QUEUEDTHREADPOOL_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_QUEUEDTHREADPOOL_H_INCLUDED

#include "Poco/SharedPtr.h"
#include "Poco/ThreadPool.h"

#include "easyhttpcpp/common/RefCountedRunnable.h"
#include "easyhttpcpp/executorservice/BlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceExports.h"

namespace easyhttpcpp {
namespace executorservice {

class EASYHTTPCPP_EXECUTORSERVICE_API QueuedThreadPool : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<QueuedThreadPool> Ptr;

    static const unsigned int DefaultCorePoolSize;
    static const unsigned int DefaultMaximumPoolSize;

    QueuedThreadPool();
    QueuedThreadPool(unsigned int corePoolSize, unsigned int maximumPoolSize);
    QueuedThreadPool(unsigned int corePoolSize, unsigned int maximumPoolSize, BlockingQueue::Ptr pWorkerQueue);
    QueuedThreadPool(BlockingQueue::Ptr pWorkerQueue);
    virtual ~QueuedThreadPool();

    virtual void start(easyhttpcpp::common::RefCountedRunnable::Ptr pTask);
    virtual easyhttpcpp::common::RefCountedRunnable::Ptr getNextTask();
    virtual void shutdown();
    virtual void shutdownAndJoinAll();
    virtual unsigned int getCorePoolSize() const;
    virtual unsigned int getMaximumPoolSize() const;

private:
    BlockingQueue::Ptr m_pBlockingQueue;
    Poco::SharedPtr<Poco::ThreadPool> m_pThreadPool;
    Poco::FastMutex m_queueMutex;
    Poco::FastMutex m_terminatedMutex;
    unsigned int m_activeWorkerCount;
    bool m_terminated;

    void initialize(unsigned int corePoolSize, unsigned int maximumPoolSize, BlockingQueue::Ptr pWorkerQueue);
    bool isTerminated();
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_QUEUEDTHREADPOOL_H_INCLUDED */
