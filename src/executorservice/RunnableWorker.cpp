/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/executorservice/BlockingQueue.h"
#include "easyhttpcpp/executorservice/QueuedThreadPool.h"

#include "RunnableWorker.h"

namespace easyhttpcpp {
namespace executorservice {

using easyhttpcpp::common::RefCountedRunnable;

static const std::string Tag = "RunnableWorker";

RunnableWorker::RunnableWorker(RefCountedRunnable::Ptr pTask, QueuedThreadPool* pQueuedThreadPool)
{
    m_pTask = pTask;
    m_pQueuedThreadPool = pQueuedThreadPool;
}

RunnableWorker::~RunnableWorker()
{
}

void RunnableWorker::run()
{
    EASYHTTPCPP_LOG_D(Tag, "run start");
    // run Runnable of this RunnableWorker
    m_pTask->run();

    // run Runnable of BlockingQueue
    while (true) {
        RefCountedRunnable::Ptr pTask = m_pQueuedThreadPool->getNextTask();
        if (!pTask) {
            break;
        }
        pTask->run();
    }
    delete this;
    EASYHTTPCPP_LOG_D(Tag, "run end");
}

} /* namespace executorservice */
} /* namespace easyhttpcpp */
