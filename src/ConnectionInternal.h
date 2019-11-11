/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CONNECTIONINTERNAL_H_INCLUDED
#define EASYHTTPCPP_CONNECTIONINTERNAL_H_INCLUDED

#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/TimerTask.h"

#include "easyhttpcpp/Connection.h"
#include "easyhttpcpp/HttpExports.h"

#include "EasyHttpContext.h"
#include "HttpTypedefs.h"

namespace easyhttpcpp {

class ConnectionPoolInternal;
class ConnectionStatusListener;
class KeepAliveTimeoutTask;

class EASYHTTPCPP_HTTP_INTERNAL_API ConnectionInternal : public Connection {
public:
    typedef Poco::AutoPtr<ConnectionInternal> Ptr;

    enum ConnectionStatus {
        Idle,               /**< idle (wait for keep-alive timeout) */
        Inuse               /**< using */
    };

    ConnectionInternal(PocoHttpClientSessionPtr pPocoHttpClientSession, const std::string& url,
            EasyHttpContext::Ptr pContext);
    virtual ~ConnectionInternal();

    virtual const std::string& getProtocol() const;

    ConnectionStatus getStatus();
    bool setInuseIfReusable(const std::string& url, EasyHttpContext::Ptr pContext);
    bool cancel();
    bool isCancelled();
    PocoHttpClientSessionPtr getPocoHttpClientSession() const;
    void setConnectionStatusListener(ConnectionStatusListener* pListener);
    bool onConnectionReleased();

    // for test
    const std::string& getScheme() const;
    const std::string& getHostName() const;
    unsigned short getHostPort() const;
    Proxy::Ptr getProxy() const;
    const std::string& getRootCaDirectory() const;
    const std::string& getRootCaFile() const;
    unsigned int getTimeoutSec() const;
    ConnectionStatusListener* getConnectionStatusListener();

private:
    Poco::FastMutex m_instanceMutex;
    Poco::FastMutex m_connectionStatusListenerMutex;
    ConnectionStatus m_connectionStatus;
    bool m_cancelled;
    PocoHttpClientSessionPtr m_pPocoHttpClientSession;
    std::string m_scheme;
    std::string m_hostName;
    unsigned short m_hostPort;
    Proxy::Ptr m_pProxy;
    std::string m_rootCaDirectory;
    std::string m_rootCaFile;
    unsigned int m_timeoutSec;
    ConnectionStatusListener* m_pConnectionStatusListener;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CONNECTIONINTERNAL_H_INCLUDED */
