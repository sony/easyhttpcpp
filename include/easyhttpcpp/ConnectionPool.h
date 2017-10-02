/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CONNECTIONPOOL_H_INCLUDED
#define EASYHTTPCPP_CONNECTIONPOOL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Connection.h"

namespace easyhttpcpp {

/**
 * @brief A ConnectionPool provide http connection pool.
 */
class ConnectionPool : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<ConnectionPool> Ptr;

    /**
     * @brief destructor
     */
    virtual ~ConnectionPool()
    {
    }

    /**
     * @brief Create ConnectionPool instance by default parameter.
     * 
     * @return ConnectionPool instance.
     */
    static ConnectionPool::Ptr createConnectionPool();

    /**
     * @brief Create ConnectionPool instance.
     * 
     * @param keepAliveIdleCountMax max connection count of keep-alive idle state in connection pool.
     * @param keepAliveTimeoutSec   keep-alive timeout second.
     * @return ConnectionPool instance.
     */
    static ConnectionPool::Ptr createConnectionPool(unsigned int keepAliveIdleCountMax,
            unsigned long keepAliveTimeoutSec);

    /**
     * @brief Get max connection count of keep-alive idle state in connection pool.
     * 
     * @return max connection count of keep-alive idle state in connection pool.
     */
    virtual unsigned int getKeepAliveIdleCountMax() const = 0;

    /**
     * @brief Get keep-alive timeout second.
     * 
     * @return keep-alive timeout second.
     */
    virtual unsigned long getKeepAliveTimeoutSec() const = 0;

    /**
     * @brief Get current connection count of keep-alive idle state in connection pool.
     * 
     * @param current connection count of keep-alive idle state in connection pool.
     */
    virtual unsigned int getKeepAliveIdleConnectionCount() = 0;

    /**
     * @brief Get current total connection count in connection pool.
     * 
     * @return current total connection count in connection pool.
     */
    virtual unsigned int getTotalConnectionCount() = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CONNECTIONPOOL_H_INCLUDED */
