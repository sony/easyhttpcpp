/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_UNBOUNDBLOCKINGQUEUE_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_UNBOUNDBLOCKINGQUEUE_H_INCLUDED

#include <queue>

#include "easyhttpcpp/executorservice/BlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceExports.h"

namespace easyhttpcpp {
namespace executorservice {

class EASYHTTPCPP_EXECUTORSERVICE_API UnboundBlockingQueue : public BlockingQueue {
public:
    UnboundBlockingQueue();
    virtual ~UnboundBlockingQueue();

    virtual bool push(easyhttpcpp::common::RefCountedRunnable::Ptr pTask);
    virtual easyhttpcpp::common::RefCountedRunnable::Ptr pop();
    virtual bool isEmpty();
    virtual void clear();

protected:
    std::queue<easyhttpcpp::common::RefCountedRunnable::Ptr> m_workerQueue;
    Poco::FastMutex m_mutex;
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_UNBOUNDBLOCKINGQUEUE_H_INCLUDED */
