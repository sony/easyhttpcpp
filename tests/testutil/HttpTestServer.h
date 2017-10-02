/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_HTTPTESTSERVER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_HTTPTESTSERVER_H_INCLUDED

#include "TestServer.h"

namespace easyhttpcpp {
namespace testutil {

class HttpTestServer : public TestServer {
public:
    HttpTestServer();
    virtual ~HttpTestServer();

protected:
    virtual Poco::Net::ServerSocket* newSocket(unsigned int port);

};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_HTTPTESTSERVER_H_INCLUDED */
