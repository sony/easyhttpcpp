/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/String.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/SSLException.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Connection.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"
#include "ConnectionPoolInternal.h"
#include "ConnectionInternal.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "ConnectionPoolInternal";

ConnectionPoolInternal::ConnectionPoolInternal(unsigned int keepAliveIdleCountMax, unsigned long keepAliveTimeoutSec) :
        m_keepAliveIdleCountMax(keepAliveIdleCountMax), m_keepAliveTimeoutSec(keepAliveTimeoutSec)
{
    EASYHTTPCPP_LOG_D(Tag, "create. this=[%p]", this);
}

ConnectionPoolInternal::~ConnectionPoolInternal()
{
    // if there are remaining ConnectionTimerTask,
    // will clear by Poco::Util::Timer::cancel and wait until clear from queue.
    m_keepAliveTimer.cancel(true);

    // clear all connection.
    m_connectionControls.clear();
}

unsigned int ConnectionPoolInternal::getKeepAliveIdleCountMax() const
{
    return m_keepAliveIdleCountMax;
}

unsigned long ConnectionPoolInternal::getKeepAliveTimeoutSec() const
{
    return m_keepAliveTimeoutSec;
}

unsigned int ConnectionPoolInternal::getKeepAliveIdleConnectionCount()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    unsigned int idleCount = 0;
    for (ConnectionControlMap::iterator itr = m_connectionControls.begin(); itr != m_connectionControls.end(); itr++) {
        ConnectionInternal::Ptr pConnectionInternal = itr->first;
        if (pConnectionInternal->getStatus() == ConnectionInternal::Idle) {
            idleCount++;
        }
    }
    return idleCount;
}

unsigned int ConnectionPoolInternal::getTotalConnectionCount()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    return static_cast<unsigned int>(m_connectionControls.size());
}

ConnectionInternal::Ptr ConnectionPoolInternal::getConnection(Request::Ptr pRequest, EasyHttpContext::Ptr pContext,
        bool& connectionReused)
{
    // find Connection.
    ConnectionInternal::Ptr pConnectionInternal = findAndReuseConnection(pRequest->getUrl(), pContext);
    if (pConnectionInternal) {
        // reuse connection.
        connectionReused = true;
        EASYHTTPCPP_LOG_D(Tag, "getConnection: reuse. this=[%p] connection=[%p]", this, pConnectionInternal.get());
        return pConnectionInternal;
    }

    // can not reuse connection.
    connectionReused = false;

    // create Connection.
    return createConnection(pRequest, pContext);
}

ConnectionInternal::Ptr ConnectionPoolInternal::createConnection(Request::Ptr pRequest, EasyHttpContext::Ptr pContext)
{
    PocoHttpClientSessionPtr pPocoHttpClientSession;
    const std::string url = pRequest->getUrl();
    Poco::URI uri;
    try {
        uri = url;

        // create Poco::Net::HTTPClientSession (HTTPSClientSession)
        if (Poco::icompare(uri.getScheme(), HttpConstants::Schemes::Https) == 0) {
            // https:
            Poco::Net::Context::Ptr pPocoContext = createPocoContext(pContext);

            pPocoHttpClientSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort(), pPocoContext);
            EASYHTTPCPP_LOG_D(Tag, "create HTTPSClientSession.");
        } else if (Poco::icompare(uri.getScheme(), HttpConstants::Schemes::Http) == 0) {
            // http:
            pPocoHttpClientSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
            EASYHTTPCPP_LOG_D(Tag, "create HTTPClientSession.");
        } else {
            EASYHTTPCPP_LOG_D(Tag, "createConnection: not supported scheme [%s]", uri.getScheme().c_str());
            throw HttpIllegalArgumentException(
                    StringUtil::format("scheme is not supported. [%s]", uri.getScheme().c_str()));
        }
        Proxy::Ptr pProxy = pContext->getProxy();
        if (pProxy) {
            pPocoHttpClientSession->setProxy(pProxy->getHost(), pProxy->getPort());
        }
        Poco::Timespan timeout(pContext->getTimeoutSec(), 0);
        pPocoHttpClientSession->setTimeout(timeout);

        pPocoHttpClientSession->setKeepAlive(true);
        pPocoHttpClientSession->setKeepAliveTimeout(getKeepAliveTimeoutForPoco());

    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createConnection: failed to create HTTPClientSessin. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpExecutionException(
                StringUtil::format("sendRequest initialization failed. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    }

    // create ConnectionInternal
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pContext);
    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);
        m_connectionControls[pConnectionInternal] = NULL;
    }

    EASYHTTPCPP_LOG_D(Tag, "createConnection: insert connection=[%p]", pConnectionInternal.get());

    return pConnectionInternal;
}

bool ConnectionPoolInternal::removeConnection(ConnectionInternal::Ptr pConnectionInternal)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    return removeConnectionWithoutLock(pConnectionInternal);
}

bool ConnectionPoolInternal::removeConnectionWithoutLock(ConnectionInternal::Ptr pConnectionInternal)
{
    ConnectionControlMap::iterator itr = m_connectionControls.find(pConnectionInternal);
    if (itr == m_connectionControls.end()) {
        EASYHTTPCPP_LOG_D(Tag, "removeConnectionWithoutLock: Connection not found. connection=[%p]",
                pConnectionInternal.get());
        return false;
    }

    m_connectionControls.erase(itr);

    EASYHTTPCPP_LOG_D(Tag, "removeConnectionWithoutLock: removed Connection from ConnectionPool. connection=[%p]",
            pConnectionInternal.get());
    return true;
}

bool ConnectionPoolInternal::releaseConnection(ConnectionInternal::Ptr pConnectionInternal)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    ConnectionControlMap::iterator itr = m_connectionControls.find(pConnectionInternal);
    if (itr == m_connectionControls.end()) {
        EASYHTTPCPP_LOG_D(Tag, "releaseConnection: Connection not found. connection=[%p]", pConnectionInternal.get());
        return false;
    }

    if (!pConnectionInternal->onConnectionReleased()) {
        EASYHTTPCPP_LOG_D(Tag, "onConnectionReleased failed. connection=[%p]", pConnectionInternal.get());
        return false;
    }

    if (pConnectionInternal->isCancelled()) {
        EASYHTTPCPP_LOG_D(Tag, "remove connection because connection is already cancelled. connection=[%p]",
                pConnectionInternal.get());
        removeConnectionWithoutLock(pConnectionInternal);
        return false;
    }

    // setup KeepAliveTimeoutTask and start timer.
    Poco::Timestamp expirationTime;
    Poco::Timespan timeoutSpan(static_cast<long>(m_keepAliveTimeoutSec), 0);
    expirationTime += timeoutSpan;
    KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = new KeepAliveTimeoutTask(expirationTime, this);
    // set KeepAliveTImeoutTask to ConnectionControlMap
    itr->second = pKeepAliveTimeoutTask;
    m_keepAliveTimer.schedule(pKeepAliveTimeoutTask, expirationTime);
    EASYHTTPCPP_LOG_D(Tag, "start KeepAliveTimeout. connection=[%p]", pConnectionInternal.get());

    // update idle connection count in connection pool
    updateConnections();

    return true;
}

void ConnectionPoolInternal::onKeepAliveTimeoutExpired(const KeepAliveTimeoutTask* pKeepAliveTimeoutTask)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (ConnectionControlMap::iterator itr = m_connectionControls.begin(); itr != m_connectionControls.end(); itr++) {
        if (itr->second.get() == pKeepAliveTimeoutTask) {
            EASYHTTPCPP_LOG_D(Tag, "onKeepAliveTimeoutExpired: removed Connection from ConnectionPool. connection=[%p]",
                    itr->first.get());

            m_connectionControls.erase(itr);
            return;
        }
    }

    EASYHTTPCPP_LOG_D(Tag,
            "onKeepAliveTimeoutExpired: connection is already removed from ConnectionPool. KeepAliveTimeoutTask=[%p]",
            pKeepAliveTimeoutTask);
}

bool ConnectionPoolInternal::isConnectionExisting(const ConnectionInternal* pConnectionInternalPtr)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    for (ConnectionControlMap::iterator itr = m_connectionControls.begin(); itr != m_connectionControls.end(); itr++) {
        if (itr->first.get() == pConnectionInternalPtr) {
            return true;
        }
    }
    return false;
}

KeepAliveTimeoutTask::Ptr ConnectionPoolInternal::getKeepAliveTimeoutTask(ConnectionInternal::Ptr pConnectionInternal)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    ConnectionControlMap::iterator itr = m_connectionControls.find(pConnectionInternal);
    if (itr == m_connectionControls.end()) {
        return NULL;
    }
    return itr->second;
}

ConnectionInternal::Ptr ConnectionPoolInternal::findAndReuseConnection(const std::string& url,
        EasyHttpContext::Ptr pContext)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    for (ConnectionControlMap::iterator itr = m_connectionControls.begin(); itr != m_connectionControls.end(); itr++) {
        ConnectionInternal::Ptr pConnectionInternal = itr->first;
        if (pConnectionInternal->setInuseIfReusable(url, pContext)) {
            // reuse connection.
            EASYHTTPCPP_LOG_D(Tag,
                    "findAndReuseConnection: Connection found and change status to Inuse connection=[%p]",
                    pConnectionInternal.get());

            // cancel KeepAliveTimeoutTask
            KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = itr->second;
            if (pKeepAliveTimeoutTask) {
                pKeepAliveTimeoutTask->cancel();
            }
            // clear KeepAliveTimeoutTask
            itr->second = NULL;

            return pConnectionInternal;
        }
    }

    return NULL;
}

void ConnectionPoolInternal::updateConnections()
{
    ConnectionInternal::Ptr pEarliestTimeoutConnection;
    KeepAliveTimeoutTask::Ptr pEarliestTimeoutKeepAliveTimeoutTask;
    unsigned int idleCount = 0;
    Poco::Timestamp earliestExpirationTime = Poco::Timestamp::TIMEVAL_MAX;
    for (ConnectionControlMap::iterator itr = m_connectionControls.begin(); itr != m_connectionControls.end(); itr++) {
        ConnectionInternal::Ptr pConnectionInternal = itr->first;
        if (pConnectionInternal->getStatus() == ConnectionInternal::Idle) {
            idleCount++;
            KeepAliveTimeoutTask::Ptr pKeepAliveTimeoutTask = itr->second;
            Poco::Timestamp& expirationTime = pKeepAliveTimeoutTask->getKeepAliveTimeoutExpirationTime();
            if (expirationTime < earliestExpirationTime) {
                earliestExpirationTime = expirationTime;
                pEarliestTimeoutConnection = itr->first;
                pEarliestTimeoutKeepAliveTimeoutTask = itr->second;
            }
        }
    }

    if (idleCount <= m_keepAliveIdleCountMax) {
        return;
    }

    pEarliestTimeoutKeepAliveTimeoutTask->cancel();
    removeConnectionWithoutLock(pEarliestTimeoutConnection);
}

Poco::Net::Context::Ptr ConnectionPoolInternal::createPocoContext(EasyHttpContext::Ptr pContext)
{
    const std::string& rootCaDirectory = pContext->getRootCaDirectory();
    const std::string& rootCaFile = pContext->getRootCaFile();
    bool isLoadDefaultCAs = false;
    if (rootCaDirectory.empty() && rootCaFile.empty()) {
        isLoadDefaultCAs = true;
    }
    try {
        // VERIFY_RELAXED is Poco::Net::Context default value.
        // VERIFY_RELAXED is SSL_VERIFY_PEER.
        Poco::Net::Context::VerificationMode verificationMode = Poco::Net::Context::VERIFY_RELAXED;
        int verificationDepth = 9; // 9 is Poco::Net::Context default value.
        Poco::Net::Context::Ptr pPocoContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,
                rootCaDirectory, verificationMode, verificationDepth, isLoadDefaultCAs);
        EASYHTTPCPP_LOG_D(Tag, "create Poco::Net::Context.");
        if (!rootCaFile.empty()) {
            // In Poco::Net:Context, only set either CaFile and CaDirectory.
            // CaFile set here.
            SSL_CTX* pSslCtx = pPocoContext->sslContext();
            if (SSL_CTX_load_verify_locations(pSslCtx, rootCaFile.c_str(), NULL) != 1) {
                EASYHTTPCPP_LOG_D(Tag, "createPocoContext: can not set CaFile. [%s]", rootCaFile.c_str());
                throw HttpSslException(StringUtil::format("can not set CaFile. [%s]", rootCaFile.c_str()));
            }
        }
        pPocoContext->disableProtocols(Poco::Net::Context::PROTO_SSLV2 | Poco::Net::Context::PROTO_SSLV3);
        return pPocoContext;
    } catch (const Poco::Net::SSLException& e) {
        EASYHTTPCPP_LOG_D(Tag, "createPocoContext: SSLException. message=[%s]", e.message().c_str());
        throw HttpSslException(StringUtil::format("SSL error occurred. message=[%s]", e.message().c_str()), e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createPocoContext: Poco::Exception occurred. message=[%s]", e.message().c_str());
        throw HttpExecutionException(StringUtil::format("sendRequest initialization error occurred. message=[%s]",
                e.message().c_str()), e);
    }
}

Poco::Timespan ConnectionPoolInternal::getKeepAliveTimeoutForPoco()
{
    // for not expired keep-alive timeout in Poco, it has a sufficiently large value than m_keepAliveTimeoutSec.
    // it is m_keepAliveTimeoutSec + 1 day;
    Poco::Timespan keepAliveTimeoutForPoco(1, 0, 0, static_cast<int>(m_keepAliveTimeoutSec), 0);
    return keepAliveTimeoutForPoco;
}

} /* namespace easyhttpcpp */
