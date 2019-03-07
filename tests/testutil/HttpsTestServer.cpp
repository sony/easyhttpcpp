/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/SSLManager.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "HttpsTestServer.h"

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "HttpsTestServer";

HttpsTestServer::HttpsTestServer() : m_defaultCaUsed(false)
{
    Poco::Net::initializeSSL();
}

HttpsTestServer::~HttpsTestServer()
{
    Poco::Net::uninitializeSSL();
}

void HttpsTestServer::setPrivateKeyFile(const Poco::Path& privateKeyFile)
{
    m_privateKeyFile = privateKeyFile;
}

void HttpsTestServer::setCertificateFile(const Poco::Path& certificateFile)
{
    m_certificateFile = certificateFile;
}

void HttpsTestServer::setCaLocation(const Poco::Path& caLocation)
{
    m_caLocation = caLocation;
}

void HttpsTestServer::setCertUnitedFile(const Poco::Path& certUnitedFile)
{
    setPrivateKeyFile(certUnitedFile);
    setCertificateFile(certUnitedFile);
    setCaLocation(certUnitedFile);
}

void HttpsTestServer::useDefaultCa(bool defaultCaUsed)
{
    m_defaultCaUsed = defaultCaUsed;
}

Poco::Net::ServerSocket* HttpsTestServer::newSocket(unsigned short port)
{
    EASYHTTPCPP_LOG_D(Tag, "newSocket: privateKeyFile=%s", m_privateKeyFile.toString().c_str());
    EASYHTTPCPP_LOG_D(Tag, "newSocket: certificateFile=%s", m_certificateFile.toString().c_str());
    EASYHTTPCPP_LOG_D(Tag, "newSocket: caLocation=%s", m_caLocation.toString().c_str());

#ifdef _WIN32
    Poco::Net::Context::Ptr pContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
            m_certificateFile.toString(),
            Poco::Net::Context::VERIFY_RELAXED,
            Poco::Net::Context::OPT_LOAD_CERT_FROM_FILE,
            Poco::Net::Context::CERT_STORE_TRUST);
    return new Poco::Net::SecureServerSocket(port, 64, pContext);
#else
    Poco::Net::Context::Ptr pContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
            m_privateKeyFile.toString(), m_certificateFile.toString(), m_caLocation.toString(),
            Poco::Net::Context::VERIFY_RELAXED, 9, m_defaultCaUsed);
    return new Poco::Net::SecureServerSocket(port, 64, pContext);
#endif
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
