/*
 * Copyright 2017 Sony Corporation
 */

#include "ConnectionConfirmationInterceptor.h"

namespace easyhttpcpp {
namespace test {

Response::Ptr ConnectionConfirmationInterceptor::intercept(Interceptor::Chain& chain)
{
    Response::Ptr pResponse = chain.proceed(chain.getRequest());
    m_pConnection = chain.getConnection();
    return pResponse;
}

Connection::Ptr ConnectionConfirmationInterceptor::getConnection()
{
    return m_pConnection;
}

void ConnectionConfirmationInterceptor::clearConnection()
{
    m_pConnection = NULL;
}

} /* namespace test */
} /* namespace easyhttpcpp */
