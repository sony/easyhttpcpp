/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_TASK_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_TASK_H_INCLUDED

#include "easyhttpcpp/common/AtomicBool.h"
#include "easyhttpcpp/common/RefCountedRunnable.h"
#include "easyhttpcpp/executorservice/ExecutorServiceExports.h"

namespace easyhttpcpp {
namespace executorservice {

class EASYHTTPCPP_EXECUTORSERVICE_API Task : public easyhttpcpp::common::RefCountedRunnable {
public:
    Task();

    virtual void runTask() = 0;

    virtual bool cancel(bool mayInterruptIfRunning);
    virtual bool isCancelled() const;
    virtual bool isDone() const;

protected:
    virtual ~Task();

    virtual void run();

private:
    Task(const Task&);
    Task& operator=(const Task&);

    easyhttpcpp::common::AtomicBool m_cancelled;
    easyhttpcpp::common::AtomicBool m_finished;
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_TASK_H_INCLUDED */
