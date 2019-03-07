/*
 * Copyright 2017 Sony Corporation
 */

#include <queue>

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/executorservice/BoundBlockingQueue.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"

namespace easyhttpcpp {
namespace executorservice {

using easyhttpcpp::common::RefCountedRunnable;

static const std::string Tag = "BoundBlockingQueue";
const unsigned int BoundBlockingQueue::DefaultMaxQueueSize = 128;

BoundBlockingQueue::BoundBlockingQueue()
{
    m_maxQueueSize = DefaultMaxQueueSize;
}

BoundBlockingQueue::BoundBlockingQueue(unsigned int maxQueueSize)
{
    if (maxQueueSize == 0) {
        std::string message = "Can not generate BoundBlockingQueue,maxQueueSize is not allow 0";
        EASYHTTPCPP_LOG_D(Tag, "Throw ExecutorServiceIllegalArgumentException : maxQueueSize = %u", maxQueueSize);
        throw ExecutorServiceIllegalArgumentException(message);
    }
    m_maxQueueSize = maxQueueSize;
}

BoundBlockingQueue::~BoundBlockingQueue()
{
}

bool BoundBlockingQueue::push(RefCountedRunnable::Ptr pTask)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    // throw exception when greater than max queue size
    if (m_maxQueueSize == m_workerQueue.size()) {
        EASYHTTPCPP_LOG_D(Tag, "BlockingQueue is full : maxQueueSize = %u", m_maxQueueSize);
        return false;
    }

    m_workerQueue.push(pTask);
    return true;
}

} /* namespace executorservice */
} /* namespace easyhttpcpp */
