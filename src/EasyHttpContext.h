/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EASYHTTPCONTEXT_H_INCLUDED
#define EASYHTTPCPP_EASYHTTPCONTEXT_H_INCLUDED

#include <list>
#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/ConnectionPool.h"
#include "easyhttpcpp/CrlCheckPolicy.h"
#include "easyhttpcpp/HttpCache.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Proxy.h"

#include "HttpExecutionTaskManager.h"

namespace easyhttpcpp {

class EasyHttpContext : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<EasyHttpContext> Ptr;
    typedef std::list<Interceptor::Ptr> InterceptorList;

    EasyHttpContext();
    virtual ~EasyHttpContext();
    virtual void setCache(HttpCache::Ptr pCache);
    virtual HttpCache::Ptr getCache() const;
    virtual void setTimeoutSec(unsigned int seconds);
    virtual unsigned int getTimeoutSec() const;
    virtual void setProxy(Proxy::Ptr pProxy);
    virtual Proxy::Ptr getProxy() const;
    virtual void setRootCaDirectory(const std::string& rootCaDirectory);
    virtual const std::string& getRootCaDirectory() const;
    virtual void setRootCaFile(const std::string& rootCaFile);
    virtual const std::string& getRootCaFile() const;
    virtual void setCrlCheckPolicy(CrlCheckPolicy crlCheckPolicy);
    virtual CrlCheckPolicy getCrlCheckPolicy() const;
    virtual void setCallInterceptors(InterceptorList& interceptors);
    virtual InterceptorList& getCallInterceptors();
    virtual void setNetworkInterceptors(InterceptorList& interceptors);
    virtual InterceptorList& getNetworkInterceptors();
    virtual void setConnectionPool(ConnectionPool::Ptr pConnectionPool);
    virtual ConnectionPool::Ptr getConnectionPool() const;
    virtual void setHttpExecutionTaskManager(HttpExecutionTaskManager::Ptr pExecutionTaskManager);
    virtual HttpExecutionTaskManager::Ptr getHttpExecutionTaskManager() const;

    static const unsigned int DefaultTimeoutSec;

private:
    HttpCache::Ptr m_pCache;
    unsigned int m_timeoutSec;
    Proxy::Ptr m_pProxy;
    std::string m_rootCaDirectory;
    std::string m_rootCaFile;
    CrlCheckPolicy m_crlCheckPolicy;
    InterceptorList m_callInterceptors;
    InterceptorList m_networkInterceptors;
    ConnectionPool::Ptr m_pConnectionPool;
    HttpExecutionTaskManager::Ptr m_pExecutionTaskManager;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EASYHTTPCONTEXT_H_INCLUDED */
