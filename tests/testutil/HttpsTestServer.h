/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_HTTPSTESTSERVER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_HTTPSTESTSERVER_H_INCLUDED

#include "Poco/Path.h"

#include "HttpTestServer.h"

namespace easyhttpcpp {
namespace testutil {


class HttpsTestServer : public TestServer {
public:
    HttpsTestServer();
    virtual ~HttpsTestServer();
    void setPrivateKeyFile(const Poco::Path& privateKeyFile);
    void setCertificateFile(const Poco::Path& certificateFile);
    void setCaLocation(const Poco::Path& caLocation);
    void setCertUnitedFile(const Poco::Path& certUnitedFile);
    void useDefaultCa(bool defaultCaUsed);

protected:
    virtual Poco::Net::ServerSocket* newSocket(unsigned short port);

private:
    Poco::Path m_privateKeyFile;
    Poco::Path m_certificateFile;
    Poco::Path m_caLocation;
    bool m_defaultCaUsed;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_HTTPSTESTSERVER_H_INCLUDED */
