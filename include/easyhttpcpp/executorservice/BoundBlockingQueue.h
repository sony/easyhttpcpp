/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_BOUNDBLOCKINGQUEUE_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_BOUNDBLOCKINGQUEUE_H_INCLUDED

#include "easyhttpcpp/executorservice/ExecutorServiceExports.h"
#include "easyhttpcpp/executorservice/UnboundBlockingQueue.h"

namespace easyhttpcpp {
namespace executorservice {

class EASYHTTPCPP_EXECUTORSERVICE_API BoundBlockingQueue : public UnboundBlockingQueue {
public:
    static const unsigned int DefaultMaxQueueSize;

    BoundBlockingQueue();
    BoundBlockingQueue(unsigned int maxQueueSize);
    virtual ~BoundBlockingQueue();

protected:
    unsigned int m_maxQueueSize;
    virtual bool push(easyhttpcpp::common::RefCountedRunnable::Ptr pTask);
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_BOUNDBLOCKINGQUEUE_H_INCLUDED */
