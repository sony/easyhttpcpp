/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CONNECTIONPOOLINTERNAL_H_INCLUDED
#define EASYHTTPCPP_CONNECTIONPOOLINTERNAL_H_INCLUDED

#include <stdint.h>
#include <map>

#include "Poco/Mutex.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/Net/Context.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"

#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/Request.h"

#include "ConnectionInternal.h"
#include "EasyHttpContext.h"
#include "KeepAliveTimeoutListener.h"
#include "KeepAliveTimeoutTask.h"

namespace easyhttpcpp {

class ConnectionPoolInternal : public ConnectionPool, public KeepAliveTimeoutListener {
public:
    typedef Poco::AutoPtr<ConnectionPoolInternal> Ptr;

    ConnectionPoolInternal(unsigned int keepAliveIdleCountMax, unsigned long keepAliveTimeoutSec);
    virtual ~ConnectionPoolInternal();

    virtual unsigned int getKeepAliveIdleCountMax() const;
    virtual unsigned long getKeepAliveTimeoutSec() const;
    virtual unsigned int getKeepAliveIdleConnectionCount();
    virtual unsigned int getTotalConnectionCount();

    virtual ConnectionInternal::Ptr getConnection(Request::Ptr pRequest, EasyHttpContext::Ptr pContext,
            bool& connectionReused);
    virtual ConnectionInternal::Ptr createConnection(Request::Ptr pRequest, EasyHttpContext::Ptr pContext);
    virtual bool removeConnection(ConnectionInternal::Ptr pConnectionInternal);
    virtual bool releaseConnection(ConnectionInternal::Ptr pConnectionInternal);

    virtual void onKeepAliveTimeoutExpired(const KeepAliveTimeoutTask* pKeepAliveTimeoutTask);

    // for test
    bool isConnectionExisting(const ConnectionInternal* pConnectionInternalPtr);
    KeepAliveTimeoutTask::Ptr getKeepAliveTimeoutTask(ConnectionInternal::Ptr pConnectionInternal);

private:
    bool removeConnectionWithoutLock(ConnectionInternal::Ptr pConnectionInternal);
    ConnectionInternal::Ptr findAndReuseConnection(const std::string& url, EasyHttpContext::Ptr pContext);
    void updateConnections();
    Poco::Timespan getKeepAliveTimeoutForPoco();

    unsigned int m_keepAliveIdleCountMax;
    unsigned long m_keepAliveTimeoutSec;
    typedef std::map<ConnectionInternal::Ptr, KeepAliveTimeoutTask::Ptr> ConnectionControlMap;
    ConnectionControlMap m_connectionControls;
    Poco::Util::Timer m_keepAliveTimer;
    Poco::FastMutex m_instanceMutex;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CONNECTIONPOOLINTERNAL_H_INCLUDED */
