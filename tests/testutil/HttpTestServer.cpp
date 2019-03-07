/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "HttpTestServer.h"
#include "HttpTestRequestHandlerFactory.h"

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestHttpServer";

HttpTestServer::HttpTestServer()
{
}

HttpTestServer::~HttpTestServer()
{
}

Poco::Net::ServerSocket* HttpTestServer::newSocket(unsigned short port)
{
    return new Poco::Net::ServerSocket(port);
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
