/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EASYHTTP_H_INCLUDED
#define EASYHTTPCPP_EASYHTTP_H_INCLUDED

#include <string>
#include <list>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/CrlCheckPolicy.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/HttpCache.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Proxy.h"
#include "easyhttpcpp/Request.h"

namespace easyhttpcpp {

/**
 * EasyHttp is an HTTP client with the following key features:
 * <ul>
 * <li>Connection pooling to reduce latency.</li>
 * <li>Multi-level (memory and file) response caching for speed and efficiency.</li>
 * <li>Ability to customize HTTP requests and responses with HTTP call interceptors.</li>
 * </ul>
 *
 * EasyHttp adapts many of its design features from a well known Android's HTTP client, okHttp and uses
 * POCO C++ libraries under the hood.
 */
class EASYHTTPCPP_HTTP_API EasyHttp : public Poco::RefCountedObject {
public:
    /**
     * A "smart" pointer to facilitate reference counting based garbage collection.
     */
    typedef Poco::AutoPtr<EasyHttp> Ptr;

    class Builder;

    virtual ~EasyHttp();

    /**
     * Create Call for HTTP request.
     *
     * @param pRequest the HTTP request.
     * @return a new Call instance.
     */
    virtual Call::Ptr newCall(Request::Ptr pRequest) = 0;

    /**
     * Get proxy settings as set inside the builder or @c NULL if not set.
     *
     * @return the proxy settings.
     */
    virtual Proxy::Ptr getProxy() const = 0;

    /**
     * Gets the HTTP request timeout in seconds as set inside the builder or 60 seconds if not set.
     *
     * This interval controls the following timeouts:
     * <ul>
     * <li>connection timeout:
     * <ul><li>the time to establish the connection with the remote host.</li></ul></li>
     * <li>socket timeout:
     * <ul><li>the time waiting for data after the connection was established. maximum time of inactivity
     * between two data packets.</li></ul></li>
     * </ul>
     *
     * @note this interval is not the timeout for total time incurred between starting an HTTP request and
     * consuming the entire response.
     *
     * @return the HTTP timeout in seconds.
     */
    virtual unsigned int getTimeoutSec() const = 0;

    /**
     * Gets the HTTP response cache object as set inside the builder or @c NULL if not set.
     *
     * @return the HTTP response cache.
     */
    virtual HttpCache::Ptr getCache() const = 0;

    /**
     * Gets the CRL (Certificate Revocation List) check policy as set inside the builder
     * or CrlCheckPolicyNoCheck if not set.
     *
     * @return the CRL check policy.
     * @see CrlCheckPolicy
     */
    virtual CrlCheckPolicy getCrlCheckPolicy() const = 0;

    /**
     * Gets the path to SSL root ca certificate directory as set inside the builder
     * or @c NULL is not set.
     *
     * @return the SSL root ca certificate directory path.
     */
    virtual const std::string& getRootCaDirectory() const = 0;

    /**
     * Gets the path to SSL root ca certificate file as set inside the builder
     * or @c NULL is not set.
     *
     * @return the SSL root ca certificate file path.
     */
    virtual const std::string& getRootCaFile() const = 0;

    /**
     * Gets the HTTP connection pool instance as set inside the builder or @c NULL if not set.
     *
     * @note EasyHttp will use a default Connection Pool in case none if provided from outside.
     *
     * @return the HTTP connection pool.
     */
    virtual ConnectionPool::Ptr getConnectionPool() const = 0;

    /**
     * Get the number of threads to keep in the thread pool to execute asynchronous request as set inside the builder
     * or 2 if not set.
     * 
     * @return the number of threads to keep.
     */
    virtual unsigned int getCorePoolSizeOfAsyncThreadPool() const = 0;

    /**
     * Get the maximum number of threads of thread pool to execute asynchronous request as set inside the builder or 5
     * if not set.
     * @return the maximum number of threads.
     */
    virtual unsigned int getMaximumPoolSizeOfAsyncThreadPool() const = 0;

    /**
     * Invalidate EasyHttp object and cancel all background tasks.
     *
     * @note please call this method from the main thread.
     */
    virtual void invalidateAndCancel() = 0;

protected:
    EasyHttp();

public:

    class EASYHTTPCPP_HTTP_API Builder {
    public:
        Builder();
        virtual ~Builder();

        /**
         * Builds EasyHttp by specified parameters.
         *
         * @return EasyHttp
         * @exception HttpExecutionException
         */
        EasyHttp::Ptr build();

        /**
         * Set Cache
         * @param pCache Cache
         * @return Builder
         */
        Builder& setCache(HttpCache::Ptr pCache);

        /**
         * @brief Get Cache
         * @return Cache
         */
        HttpCache::Ptr getCache() const;

        /**
         * @brief Set Http timeout.
         * @param timeout timeout value (sec)
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setTimeoutSec(unsigned int timeoutSec);

        /**
         * @brief Get Http timeout.
         * @return timeout (sec)
         */
        unsigned int getTimeoutSec() const;

        /**
         * @brief Set Proxy
         * @param pProxy Proxy
         * @return Builder
         */
        Builder& setProxy(Proxy::Ptr pProxy);

        /**
         * @brief Get Proxy
         * @return Proxy
         */
        Proxy::Ptr getProxy() const;

        /**
         * @brief Set Root CA directory.
         * 
         * if root CA directory and file is no set, use OpenSSL default root CA.
         * @param rootCaDirectory root CA directory
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setRootCaDirectory(const std::string& rootCaDirectory);

        /**
         * @brief Get root CA directory.
         * @return root CA directory
         */
        const std::string& getRootCaDirectory() const;

        /**
         * @brief Set Root CA file.
         * 
         * if root CA directory and file is no set, use OpenSSL default root CA.
         * @param rootCaFile root CA file
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setRootCaFile(const std::string& rootCaFile);

        /**
         * @brief Get root CA file.
         * @return root CA file
         */
        const std::string& getRootCaFile() const;

        /**
         * @brief Set CrlCheckPolicy
         * 
         * if CrlCheckPolucy is not set, use CrlCheckPolicyNoCheck.
         * @param crlCheckPolicy CrlCheckPolicy
         * @return Builder
         */
        Builder& setCrlCheckPolicy(CrlCheckPolicy crlCheckPolicy);

        /**
         * @brief Get CrlCheckPolicy
         * @return CrlCheckPolicy
         */
        CrlCheckPolicy getCrlCheckPolicy() const;

        /**
         * @brief Add CallInterceptor.
         * @param pInterceptor CallInterceptor
         * @return Builder
         */
        Builder& addInterceptor(Interceptor::Ptr pInterceptor);

        /**
         * @brief Get CallInterceptor list.
         * @return CallInterceptor list
         */
        std::list<Interceptor::Ptr>& getInterceptors();

        /**
         * @brief Add NetworkInterceptor.
         * @param pInterceptor NetwoekInterceptor
         * @return Builder
         */
        Builder& addNetworkInterceptor(Interceptor::Ptr pInterceptor);

        /**
         * @brief Get NetworkInterceptor list.
         * @return NetworkInterceptor list
         */
        std::list<Interceptor::Ptr>& getNetworkInterceptors();

        /**
         * @brief Set ConnectionPool
         * @param pConnectionPool ConnectionPool
         * @return Builder
         */
        Builder& setConnectionPool(ConnectionPool::Ptr pConnectionPool);

        /**
         * @brief Get ConnectionPool.
         * @return ConnectionPool
         */
        ConnectionPool::Ptr getConnectionPool() const;

        /**
         * @brief Set the number of threads to keep in the thread pool to execute asynchronous request,
         * even if they are idle.
         * 
         * @param corePoolSizeOfAsyncThreadPool the number of threads to keep.
         * @return Builder
         */
        Builder& setCorePoolSizeOfAsyncThreadPool(unsigned int corePoolSizeOfAsyncThreadPool);

        /**
         * @brief Get the number of threads to keep in the thread pool to execute asynchronous request,
         * even if they are idle.
         * 
         * If the number of threads to keep is not set, the number of threads to keep is 2.
         * @return the number of threads to keep.
         */
        unsigned int getCorePoolSizeOfAsyncThreadPool() const;

        /**
         * @brief Set the maximum number of threads of thread pool to execute asynchronous request.
         * 
         * If asynchronous requests greater than the maximum size are requested, the request is queued and executed
         * sequentially.
         * @param maximumPoolSizeOfAsyncThreadPool the maximum number of threads.
         * @return Builder
         */
        Builder& setMaximumPoolSizeOfAsyncThreadPool(unsigned int maximumPoolSizeOfAsyncThreadPool);

        /**
         * @brief Get the maximum number of threads of thread pool to execute asynchronous request.
         * 
         * If the maximum number of threads is not set, the maximum number of threads is 5.
         * @return the maximum number of threads.
         */
        unsigned int getMaximumPoolSizeOfAsyncThreadPool() const;

    private:
        HttpCache::Ptr m_pCache;
        unsigned int m_timeoutSec;
        Proxy::Ptr m_pProxy;
        std::string m_rootCaDirectory;
        std::string m_rootCaFile;
        CrlCheckPolicy m_crlCheckPolicy;
        std::list<Interceptor::Ptr> m_callInterceptors;
        std::list<Interceptor::Ptr> m_networkInterceptors;
        ConnectionPool::Ptr m_pConnectionPool;
        unsigned int m_corePoolSizeOfAsyncThreadPool;
        unsigned int m_maximumPoolSizeOfAsyncThreadPool;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EASYHTTP_H_INCLUDED */
