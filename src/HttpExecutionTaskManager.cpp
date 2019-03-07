/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/CommonException.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"
#include "easyhttpcpp/executorservice/UnboundBlockingQueue.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpExecutionTaskManager.h"

using easyhttpcpp::common::FutureCancellationException;
using easyhttpcpp::common::FutureExecutionException;
using easyhttpcpp::executorservice::ExecutorServiceException;
using easyhttpcpp::executorservice::QueuedThreadPool;
using easyhttpcpp::executorservice::UnboundBlockingQueue;

namespace easyhttpcpp {

static const std::string Tag = "HttpExecutionTaskManager";

HttpExecutionTaskManager::HttpExecutionTaskManager(unsigned int corePoolSizeOfAsyncThreadPool,
        unsigned int maximumPoolSizeOfAsyncThreadPool) : m_terminated(false)
{
    try {
        m_pAsyncThreadPool = new QueuedThreadPool(corePoolSizeOfAsyncThreadPool,
                maximumPoolSizeOfAsyncThreadPool, new UnboundBlockingQueue());
    } catch (const ExecutorServiceException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Can not create QueuedThreadPool. Details:%s", e.getMessage().c_str());
        throw HttpExecutionException("Can not create asynchronous thread pool. Check getCause() for details.", e);
    }
}

HttpExecutionTaskManager::~HttpExecutionTaskManager()
{
}

void HttpExecutionTaskManager::start(HttpExecutionTask::Ptr pExecutionTask)
{
    try {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);

        if (m_terminated) {
            EASYHTTPCPP_LOG_D(Tag, "HttpExecutionTaskManager was already terminated");
            throw HttpIllegalStateException("Failed to execute task, because EasyHttp was invalidated.");
        }

        if (!m_pAsyncThreadPool) {
            EASYHTTPCPP_LOG_D(Tag, "HttpExecutionTaskManager was terminated");
            throw HttpIllegalStateException("Failed to execute task, because EasyHttp was released.");
        }

        m_executionTaskList.push_back(pExecutionTask);
        EASYHTTPCPP_LOG_D(Tag, "add HttpExecutionTask.[%p]", pExecutionTask.get());

        m_pAsyncThreadPool->start(pExecutionTask);
    } catch (const ExecutorServiceException& e) {
        removeTask(pExecutionTask);
        EASYHTTPCPP_LOG_D(Tag, "QueuedThreadPool::start failed. Details:%s", e.getMessage().c_str());
        throw HttpExecutionException("Can not start asynchronous execute. getCause() for details.", e);
    }
}

void HttpExecutionTaskManager::gracefulShutdown()
{
    ExecutionList executionTaskListCopy;
    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);

        if (m_terminated) {
            EASYHTTPCPP_LOG_D(Tag, "HttpExecutionTaskManager is already terminated.");
            return;
        }
        m_terminated = true;

        // after shutdown, can not execute start method.
        m_pAsyncThreadPool->shutdown();

        executionTaskListCopy = m_executionTaskList;
    }

    // cancel all task
    for (ExecutionList::iterator itr = executionTaskListCopy.begin(); itr != executionTaskListCopy.end(); itr++) {
        (*itr)->cancel(true);
    }

    // wait for all task to complete.
    for (ExecutionList::iterator itr = executionTaskListCopy.begin(); itr != executionTaskListCopy.end(); itr++) {
        try {
            // wait task.
            (*itr)->get();
        } catch (const FutureCancellationException& e) {
            EASYHTTPCPP_LOG_D(Tag, "HttpExecutionTask got cancelled successfully. Details: %s", e.getMessage().c_str());
            // ignore FutureCancellationException.
        } catch (const FutureExecutionException& e) {
            EASYHTTPCPP_LOG_D(Tag, "HttpExecutionTask completed with error while waiting for completion. Details: %s",
                    e.getMessage().c_str());
            // ignore FutureExecutionException.
        }
    }

    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);
        // wait for all threads to finish completely
        m_pAsyncThreadPool->shutdownAndJoinAll();
        // release QueuedThreadPool.
        m_pAsyncThreadPool = NULL;
    }
}

void HttpExecutionTaskManager::onComplete(HttpExecutionTask::Ptr pExecutionTask)
{
    removeTask(pExecutionTask);
}

void HttpExecutionTaskManager::removeTask(HttpExecutionTask::Ptr pExecutionTask)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    for (ExecutionList::iterator itr = m_executionTaskList.begin();
            itr != m_executionTaskList.end(); itr++) {
        if (*itr == pExecutionTask) {
            EASYHTTPCPP_LOG_D(Tag, "remove HttpExecutionTask.[%p]", pExecutionTask.get());
            m_executionTaskList.erase(itr);
            break;
        }
    }
}

} /* namespace easyhttpcpp */
