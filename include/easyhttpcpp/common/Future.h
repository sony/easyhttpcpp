/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_FUTURE_H_INCLUDED
#define EASYHTTPCPP_COMMON_FUTURE_H_INCLUDED

namespace easyhttpcpp {
namespace common {

template<class Result>
class Future {
public:
    virtual ~Future()
    {
    }

    /**
     * Attempts to cancel execution of the task. Instead of blocking till the task actually stops, this method just
     * propagates the cancel message to the running task. Depending upon when the task actually receives this cancel
     * message, the running task might take some time to stop completely.
     *
     * <p>
     * This attempt will fail if the task has already completed, has already been cancelled, or could not be cancelled
     * for some other reason. If successful, and this task has not yet been executed when cancel is called, this task
     * should never run. If the task has already started, then the @c mayInterruptIfRunning parameter determines
     * whether the thread executing this task should be cancelled in an attempt to stop the task. Note that even after
     * successfully cancelling a running task (i.e. this method returns @c true), there might be cases when the
     * task finishes before receiving the cancel message. In such cases, the task cannot be cancelled and result
     * will be returned just like a normal task execution.
     * </p>
     *
     * @param mayInterruptIfRunning @c true if the thread executing this task should be cancelled;
     *         otherwise, in-progress tasks are allowed to complete.
     * @return false if the task could not be cancelled; true otherwise.
     */
    virtual bool cancel(bool mayInterruptIfRunning) = 0;

    /**
     * Returns true if the task was cancelled before it completed normally. This method will always return true if
     * #cancel(bool) returned true;
     *
     * @return true if the task was cancelled before it completed.
     */
    virtual bool isCancelled() const = 0;

    /**
     * Returns true if the task completed. Completion maybe due to normal termination, an exception or cancellation --
     * in all of these cases, this method returns true.
     *
     * @return true if this task completed.
     */
    virtual bool isDone() const = 0;

    /**
     * Waits if necessary for at most the given time for the computation to complete, and then retrieves its result,
     * if available.
     *
     * @return the computed result.
     * @throw FutureCancellationException if the computation was cancelled.
     * @throw FutureExecutionException if the computation threw an exception.
     */
    virtual Result get() = 0;

    /**
     * Waits if necessary for at most the given time for the computation to complete, and then retrieves its result,
     * if available.
     *
     * @param timeoutMillis the maximum time to wait in milliseconds.
     * @return the computed result.
     * @throw FutureCancellationException if the computation was cancelled.
     * @throw FutureExecutionException if the computation threw an exception.
     * @throw FutureTimeoutException if the wait timed out.
     */
    virtual Result get(unsigned long timeoutMillis) = 0;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_FUTURE_H_INCLUDED */
