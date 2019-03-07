/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPEXECUTIONTASK_H_INCLUDED
#define EASYHTTPCPP_HTTPEXECUTIONTASK_H_INCLUDED

#include "Poco/Void.h"

#include "easyhttpcpp/executorservice/FutureTask.h"

namespace easyhttpcpp {

class HttpExecutionTaskListener;

class HttpExecutionTask : public easyhttpcpp::executorservice::FutureTask<Poco::Void> {
public:
    typedef Poco::AutoPtr<HttpExecutionTask> Ptr;

    virtual ~HttpExecutionTask()
    {
    }
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPEXECUTIONTASK_H_INCLUDED */
