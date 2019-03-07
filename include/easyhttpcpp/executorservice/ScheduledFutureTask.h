/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_SCHEDULEDFUTURETASK_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_SCHEDULEDFUTURETASK_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Event.h"
#include "Poco/Exception.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Util/TimerTask.h"

#include "easyhttpcpp/common/AtomicBool.h"
#include "easyhttpcpp/common/CommonException.h"
#include "easyhttpcpp/common/Future.h"
#include "easyhttpcpp/common/RefCountedRunnable.h"
#include "easyhttpcpp/executorservice/ExecutorServiceException.h"

namespace easyhttpcpp {
namespace executorservice {

template<class Result>
class ScheduledFutureTask : public Poco::Util::TimerTask, public easyhttpcpp::common::Future<Result> {
public:
    typedef Poco::AutoPtr<ScheduledFutureTask<Result> > Ptr;

    ScheduledFutureTask() : m_finishedEvent(false), m_cancelled(false), m_finished(false)
    {
    }

    virtual ~ScheduledFutureTask()
    {
    }

    virtual void runTask() = 0;

    virtual bool cancel(bool mayInterruptIfRunning)
    {
        // classes overriding may use mayInterruptIfRunning; base implementation does nothing

        m_cancelled.set(true);
        return true;
    }

    virtual bool isCancelled() const
    {
        return m_cancelled.get();
    }

    virtual bool isDone() const
    {
        return m_finished.get();
    }

    virtual Result get()
    {
        // wait assuming runTask() will eventually be called at some point of time
        try {
            // will return immediately if the event was already set
            m_finishedEvent.wait();
        } catch (const Poco::Exception& e) {
            throw easyhttpcpp::common::FutureExecutionException("Internal error occurred while waiting for result. "
                    "Check getCause() for details.", e);
        }

        // check if cancelled
        if (isCancelled()) {
            throw easyhttpcpp::common::FutureCancellationException("Unable to get result since already cancelled.");
        }

        // throw exception if task computation resulted in error
        if (isFailed()) {
            // wrap the exception in ExecutionException
            throw easyhttpcpp::common::FutureExecutionException("Failed to get result. "
                    "Check getCause() for actual error.", *m_pException.get());
        }

        return m_result;
    }

    virtual Result get(unsigned long timeoutMillis)
    {
        // wait assuming runTask() will eventually be called at some point of time
        try {
            // will return immediately if the event was already set
            m_finishedEvent.wait(timeoutMillis);
        } catch (const Poco::TimeoutException& e) {
            throw easyhttpcpp::common::FutureTimeoutException("Unable to get result since wait timed out. "
                    "Check getCause() for details.", e);
        } catch (const Poco::Exception& e) {
            throw easyhttpcpp::common::FutureExecutionException("Internal error occurred while waiting for result. "
                    "Check getCause() for details.", e);
        }

        // check if cancelled
        if (isCancelled()) {
            throw easyhttpcpp::common::FutureCancellationException("Unable to get result since already cancelled.");
        }

        // throw exception if task computation resulted in error
        if (isFailed()) {
            // wrap the exception in ExecutionException
            throw easyhttpcpp::common::FutureExecutionException("Failed to get result. "
                    "Check getCause() for actual error.", *m_pException.get());
        }

        return m_result;
    }

protected:
    virtual void run()
    {
        Poco::Util::TimerTask::Ptr guard(this, false); // ensure automatic release when done
        try {
            // result will be set by the class implementing runTask()
            runTask();
        } catch (const easyhttpcpp::common::BaseException& e) {
            setException(e.clone());
        } catch (const std::exception& e) {
            setException(
                    new ExecutorServiceExecutionException("Internal error occurred. Check getCause() for details.", e));
        } catch (...) {
            setException(new ExecutorServiceExecutionException("Unexpected internal error occurred."));
        }

        m_finished.set(true);
        // notify the waiting threads that the result became available.
        notify();
    }

    virtual void setException(easyhttpcpp::common::BaseException::Ptr pException)
    {
        Poco::FastMutex::ScopedLock lock(m_resultMutex);

        m_pException = pException;
    }

    virtual void setResult(Result result)
    {
        Poco::FastMutex::ScopedLock lock(m_resultMutex);

        m_result = result;
    }

    virtual void notify()
    {
        m_finishedEvent.set();
    }

private:
    mutable Poco::FastMutex m_resultMutex;
    Result m_result;
    easyhttpcpp::common::BaseException::Ptr m_pException;
    Poco::Event m_finishedEvent;
    easyhttpcpp::common::AtomicBool m_cancelled;
    easyhttpcpp::common::AtomicBool m_finished;

    virtual bool isFailed() const
    {
        Poco::FastMutex::ScopedLock lock(m_resultMutex);

        return !m_pException.isNull();
    }
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif //EASYHTTPCPP_EXECUTORSERVICE_SCHEDULEDFUTURETASK_H_INCLUDED
