/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPASYNCEXECUTIONTASK_H_INCLUDED
#define EASYHTTPCPP_HTTPASYNCEXECUTIONTASK_H_INCLUDED

#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseCallback.h"

#include "EasyHttpContext.h"
#include "HttpExecutionTask.h"
#include "HttpRequestExecutor.h"

namespace easyhttpcpp {

class HttpAsyncExecutionTask : public HttpExecutionTask {
public:
    typedef Poco::AutoPtr<HttpAsyncExecutionTask> Ptr;
    HttpAsyncExecutionTask(EasyHttpContext::Ptr pContext, HttpRequestExecutor::Ptr pRequestExecutor,
            ResponseCallback::Ptr pResponseCallback);
    virtual ~HttpAsyncExecutionTask();

    virtual void runTask();
    virtual bool cancel(bool mayInterruptIfRunning);
    virtual bool isCancelled() const;

private:
    HttpAsyncExecutionTask();
    void notifyCompletion(HttpException::Ptr pWhat, Response::Ptr pResponse);

    EasyHttpContext::Ptr m_pContext;
    HttpRequestExecutor::Ptr m_pRequestExecutor;
    ResponseCallback::Ptr m_pResponseCallback;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPASYNCEXECUTIONTASK_H_INCLUDED */
