/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_CONNECTIONCONFIRMATIONINTERCEPTOR_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_CONNECTIONCONFIRMATIONINTERCEPTOR_H_INCLUDED

#include "Poco/AutoPtr.h"

#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Connection.h"

namespace easyhttpcpp {
namespace test {

class ConnectionConfirmationInterceptor : public Interceptor {
public:
    typedef Poco::AutoPtr<ConnectionConfirmationInterceptor> Ptr;

    Response::Ptr intercept(Interceptor::Chain& chain);
    Connection::Ptr getConnection();
    void clearConnection();
private:
    Connection::Ptr m_pConnection;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_CONNECTIONCONFIRMATIONINTERCEPTOR_H_INCLUDED */
