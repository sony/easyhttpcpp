/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTSERVER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTSERVER_H_INCLUDED

#include "Poco/SharedPtr.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"

#include "HttpTestRequestHandlerFactory.h"
#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

class HttpTestRequestHandlerFactory;

class EASYHTTPCPP_TESTUTIL_API TestServer {
public:
    TestServer();
    virtual ~TestServer();
    void start(unsigned short port);
    void stop();
    HttpTestRequestHandlerFactory& getTestRequestHandlerFactory();
    void setKeepAlive(bool keepAlive);
    void setKeepAliveTimeoutSec(long keepAliveTimeoutSec);

protected:
    virtual Poco::Net::ServerSocket* newSocket(unsigned short port) = 0;

private:
    Poco::Net::HTTPRequestHandlerFactory::Ptr m_pRequestHandlerFactory;
    Poco::SharedPtr<Poco::Net::ServerSocket> m_pSocket;
    Poco::SharedPtr<Poco::Net::HTTPServer> m_pServer;
    bool m_keepAlive;
    long m_keepAliveTimeoutSec;

};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESUTUTIL_TESTSERVER_H_INCLUDED */
