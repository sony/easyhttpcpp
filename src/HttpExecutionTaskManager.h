/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPEXECUTIONTASKMANAGER_H_INCLUDED
#define EASYHTTPCPP_HTTPEXECUTIONTASKMANAGER_H_INCLUDED

#include <list>

#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/executorservice/QueuedThreadPool.h"

#include "HttpExecutionTask.h"

namespace easyhttpcpp {

class HttpExecutionTaskManager : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpExecutionTaskManager> Ptr;

    HttpExecutionTaskManager(unsigned int corePoolSizeOfAsyncThreadPool, unsigned int maximumPoolSizeOfAsyncThreadPool);
    virtual ~HttpExecutionTaskManager();
    virtual void start(HttpExecutionTask::Ptr pExecutionTask);
    virtual void gracefulShutdown();

    void onComplete(HttpExecutionTask::Ptr pExecutionTask);

private:
    HttpExecutionTaskManager();
    void removeTask(HttpExecutionTask::Ptr pExecutionTask);

    easyhttpcpp::executorservice::QueuedThreadPool::Ptr m_pAsyncThreadPool;
    Poco::FastMutex m_instanceMutex;
    typedef std::list<HttpExecutionTask::Ptr> ExecutionList;
    ExecutionList m_executionTaskList;
    bool m_terminated;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPEXECUTIONTASKMANAGER_H_INCLUDED */
