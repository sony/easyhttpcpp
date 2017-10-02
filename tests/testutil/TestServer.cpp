/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "TestServer.h"
#include "HttpTestRequestHandlerFactory.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestServer";
static const long DefaultKeepAliveTimeoutSec = 15;  // 15 sec is default value of Poco::Net::HTTPServerParams.

TestServer::TestServer() : m_pRequestHandlerFactory(new HttpTestRequestHandlerFactory()), m_keepAlive(true),
        m_keepAliveTimeoutSec(DefaultKeepAliveTimeoutSec)
{
}

TestServer::~TestServer()
{
    stop();
}

void TestServer::start(unsigned int port)
{
    int maxQueued = 100;
    int maxThreads = 16;

    Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
    pParams->setMaxQueued(maxQueued);
    pParams->setMaxThreads(maxThreads);
    pParams->setKeepAlive(m_keepAlive);
    pParams->setKeepAliveTimeout(Poco::Timespan(m_keepAliveTimeoutSec, 0));

    // set-up a server socket
    m_pSocket = newSocket(port);
    // set-up a HTTPServer instance
    m_pServer = new Poco::Net::HTTPServer(m_pRequestHandlerFactory, *m_pSocket, pParams);
    // start the HTTPServer
    m_pServer->start();
    EASYHTTPCPP_LOG_D(Tag, "start server");
}

void TestServer::stop()
{
    m_pServer->stopAll(true);
    EASYHTTPCPP_LOG_D(Tag, "stop server");
}

HttpTestRequestHandlerFactory& TestServer::getTestRequestHandlerFactory()
{
    return *(static_cast<HttpTestRequestHandlerFactory*> (m_pRequestHandlerFactory.get()));
}

void TestServer::setKeepAlive(bool keepAlive)
{
    m_keepAlive = keepAlive;
    EASYHTTPCPP_LOG_D(Tag, "setKeepAlive [%s]", StringUtil::boolToString(m_keepAlive));
}

void TestServer::setKeepAliveTimeoutSec(long keepAliveTimeoutSec)
{
    m_keepAliveTimeoutSec = keepAliveTimeoutSec;
    EASYHTTPCPP_LOG_D(Tag, "set keepAliveTimeoutSec [%ld]", m_keepAliveTimeoutSec);
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
