/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTRESPONSECALLBACK_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTRESPONSECALLBACK_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Event.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/ResponseCallback.h"

namespace easyhttpcpp {
namespace test {

class HttpTestResponseCallback : public ResponseCallback {
public:
    typedef Poco::AutoPtr<HttpTestResponseCallback> Ptr;

    HttpTestResponseCallback();
    virtual void onResponse(Response::Ptr pResponse);
    virtual void onFailure(HttpException::Ptr pWhat);
    Response::Ptr getResponse();
    HttpException::Ptr getWhat();
    bool waitCompletion();
    bool waitCompletion(long timeoutMilliSec);
    Poco::Timestamp getCompletionTime() const;

private:
    Response::Ptr m_pResponse;
    HttpException::Ptr m_pWhat;
    Poco::Event m_completionEvent;
    Poco::Timestamp m_completionTime;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTRESPONSECALLBACK_H_INCLUDED */
