/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_RUNNABLEWORKER_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_RUNNABLEWORKER_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"

namespace easyhttpcpp {
namespace executorservice {

class QueuedThreadPool;

class RunnableWorker : public Poco::Runnable {
public:

    RunnableWorker(easyhttpcpp::common::RefCountedRunnable::Ptr pTask, QueuedThreadPool* pQueuedThreadPool);
    virtual ~RunnableWorker();

    virtual void run();
private:
    easyhttpcpp::common::RefCountedRunnable::Ptr m_pTask;
    QueuedThreadPool* m_pQueuedThreadPool;
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_RUNNABLEWORKER_H_INCLUDED */
