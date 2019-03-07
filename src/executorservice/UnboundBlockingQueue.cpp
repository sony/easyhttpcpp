/*
 * Copyright 2017 Sony Corporation
 */

#include <queue>

#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"

#include "easyhttpcpp/executorservice/UnboundBlockingQueue.h"

namespace easyhttpcpp {
namespace executorservice {

using easyhttpcpp::common::RefCountedRunnable;

static const std::string Tag = "UnboundBlockingQueue";

UnboundBlockingQueue::UnboundBlockingQueue()
{
}

UnboundBlockingQueue::~UnboundBlockingQueue()
{
    clear();
}

bool UnboundBlockingQueue::push(RefCountedRunnable::Ptr pTask)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    m_workerQueue.push(pTask);
    return true;
}

RefCountedRunnable::Ptr UnboundBlockingQueue::pop()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    if (m_workerQueue.empty()) {
        return NULL;
    }
    RefCountedRunnable::Ptr pTask = m_workerQueue.front();
    m_workerQueue.pop();
    return pTask;
}

bool UnboundBlockingQueue::isEmpty()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    return m_workerQueue.empty();
}

void UnboundBlockingQueue::clear()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    while (!m_workerQueue.empty()) {
        m_workerQueue.pop();
    }
}

} /* namespace executorservice */
} /* namespace easyhttpcpp */
