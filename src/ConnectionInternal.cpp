/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Mutex.h"
#include "Poco/String.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPMessage.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"

#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "ConnectionStatusListener.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "ConnectionInternal";

ConnectionInternal::ConnectionInternal(PocoHttpClientSessionPtr pPocoHttpClientSession, const std::string& url,
        EasyHttpContext::Ptr pContext) : m_connectionStatus(Inuse), m_cancelled(false),
        m_pPocoHttpClientSession(pPocoHttpClientSession), m_hostPort(0), m_timeoutSec(0),
        m_pConnectionStatusListener(NULL)
{
    EASYHTTPCPP_LOG_D(Tag, "create this=[%p] url=[%s]", this, url.c_str());

    try {
        Poco::URI uri(url);
        m_scheme = uri.getScheme();
        m_hostName = uri.getHost();
        m_hostPort = uri.getPort();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "ConnectionInternal: invalid url[%s] message=%s", url.c_str(), e.message().c_str());
        throw HttpExecutionException(StringUtil::format(
                "url is not valid. [%s] message=[%s]", url.c_str(), e.message().c_str()), e);
    }
    m_pProxy = pContext->getProxy();
    m_rootCaDirectory = pContext->getRootCaDirectory();
    m_rootCaFile = pContext->getRootCaFile();
    m_timeoutSec = pContext->getTimeoutSec();
}

ConnectionInternal::~ConnectionInternal()
{
}

const std::string& ConnectionInternal::getProtocol() const
{
    return Poco::Net::HTTPMessage::HTTP_1_1;
}

ConnectionInternal::ConnectionStatus ConnectionInternal::getStatus()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    return m_connectionStatus;
}

bool ConnectionInternal::setInuseIfReusable(const std::string& url, EasyHttpContext::Ptr pContext)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (m_connectionStatus != Idle) {
        return false;
    }

    // check scheme, host name m host port
    try {
        Poco::URI uri(url);
        if (Poco::icompare(uri.getScheme(), m_scheme) != 0) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: scheme is different. request=[%s] target=[%s]",
                    uri.getScheme().c_str(), m_scheme.c_str());
            return false;
        }
        if (uri.getHost() != m_hostName) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: host name is different. request=[%s] target=[%s]",
                    uri.getHost().c_str(), m_hostName.c_str());
            return false;
        }
        if (uri.getPort() != m_hostPort) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: port is different. request=[%u] target=[%u]",
                    uri.getPort(), m_hostPort);
            return false;
        }
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "setInuseIfReusable: invalid url[%s] message=%s", url.c_str(), e.message().c_str());
        return false;
    }

    // check proxy
    Proxy::Ptr pProxy = pContext->getProxy();
    if (pProxy) {
        if (!m_pProxy) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: no proxy. request=[%s]%u]",
                    pProxy->getHost().c_str(), pProxy->getPort());
            return false;
        }
        if (*pProxy != *m_pProxy) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: proxy name is different. request=[%s:%u] target=[%s:%u]",
                    pProxy->getHost().c_str(), pProxy->getPort(), m_pProxy->getHost().c_str(), m_pProxy->getPort());
            return false;
        }
    } else {
        if (m_pProxy) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: proxy exist. target=[%s:%u]",
                    m_pProxy->getHost().c_str(), m_pProxy->getPort());
            return false;
        }
    }

    // check rootCa
    if (Poco::icompare(m_scheme, HttpConstants::Schemes::Https) == 0) {
        if (pContext->getRootCaDirectory() != m_rootCaDirectory) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: rooCaDirectory is different. request=[%s] target=[%s]",
                    pContext->getRootCaDirectory().c_str(), m_rootCaDirectory.c_str());
            return false;
        }
        if (pContext->getRootCaFile() != m_rootCaFile) {
            EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: rooCaFile is different. request=[%s] target=[%s]",
                    pContext->getRootCaFile().c_str(), m_rootCaFile.c_str());
            return false;
        }
    }

    // check timeout sec.
    if (pContext->getTimeoutSec() != m_timeoutSec) {
        EASYHTTPCPP_LOG_V(Tag, "setInuseIfReusable: timeoutSec is different. request=[%u] target=[%u]",
                pContext->getTimeoutSec(), m_timeoutSec);
        return false;
    }

    EASYHTTPCPP_LOG_D(Tag, "setInuseIfReusable: reuse Connection and change status to Inuse.");
    m_connectionStatus = Inuse;

    return true;
}

bool ConnectionInternal::cancel()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (m_cancelled) {
        EASYHTTPCPP_LOG_D(Tag, "cancel: already cancelled");
        return true;
    }

    bool ret = true;
    m_cancelled = true;
    EASYHTTPCPP_LOG_D(Tag, "cancel: cancelled");
    if (m_pPocoHttpClientSession) {
        try {
#ifdef WIN32
            // Windows can not close socket with shutdown; We also need to call socket::close, so we call
            // HttpClientSession::abort.
            EASYHTTPCPP_LOG_D(Tag, "cancel: call HTTPClientSession::abort.");
            m_pPocoHttpClientSession->abort();
#else
            // Since communication can be interrupted by calling Socket::shutdown, no need to call
            // Socket::close on Linux.
            EASYHTTPCPP_LOG_D(Tag, "cancel: call HTTPClientSession::socket::shutdown.");
            m_pPocoHttpClientSession->socket().shutdown();
#endif
        } catch (const Poco::Exception& e) {
            EASYHTTPCPP_LOG_D(Tag, "cancel: HTTPClientSession::abort throws PocoException [%s]", e.message().c_str());
            ret = false;
        } catch (const std::exception& e) {
            EASYHTTPCPP_LOG_D(Tag, "cancel: HTTPClientSession::abort throws std::exception [%s]", e.what());
            ret = false;
        }
    }

    // When releaseConnection, create KeepAliveTimeoutTask and clear reference of ConnectionInternal from HttpEngine.
    // Therefore, there is no KeepAliveTimeoutTask when cancel is called from HttpEngine.
    // It is not necessary to call m_pKeepAliveTimeoutTask->cancel().

    return ret;
}

bool ConnectionInternal::isCancelled()
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    return m_cancelled;
}

PocoHttpClientSessionPtr ConnectionInternal::getPocoHttpClientSession() const
{
    return m_pPocoHttpClientSession;
}

void ConnectionInternal::setConnectionStatusListener(ConnectionStatusListener* pListener)
{
    Poco::FastMutex::ScopedLock lock(m_connectionStatusListenerMutex);
    m_pConnectionStatusListener = pListener;
}

bool ConnectionInternal::onConnectionReleased()
{
    {
        Poco::FastMutex::ScopedLock lock(m_connectionStatusListenerMutex);
        if (m_pConnectionStatusListener) {
            bool listenerInvalidated = false;
            m_pConnectionStatusListener->onIdle(this, listenerInvalidated);
            if (listenerInvalidated) {
                m_pConnectionStatusListener = NULL;
            }
        }
    }

    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);
        EASYHTTPCPP_LOG_D(Tag, "releaseConnection: change status to Idle");
        if (m_connectionStatus == Idle) {
            EASYHTTPCPP_LOG_D(Tag, "releaseConnection: status is already Idle.");
            return false;
        }
        m_connectionStatus = Idle;
    }
    return true;
}

const std::string& ConnectionInternal::getScheme() const
{
    return m_scheme;
}

const std::string& ConnectionInternal::getHostName() const
{
    return m_hostName;
}

unsigned short ConnectionInternal::getHostPort() const
{
    return m_hostPort;
}

Proxy::Ptr ConnectionInternal::getProxy() const
{
    return m_pProxy;
}

const std::string& ConnectionInternal::getRootCaDirectory() const
{
    return m_rootCaDirectory;
}

const std::string& ConnectionInternal::getRootCaFile() const
{
    return m_rootCaFile;
}

unsigned int ConnectionInternal::getTimeoutSec() const
{
    return m_timeoutSec;
}

ConnectionStatusListener* ConnectionInternal::getConnectionStatusListener()
{
    return m_pConnectionStatusListener;
}

} /* namespace easyhttpcpp */
