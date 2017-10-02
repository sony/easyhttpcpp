/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/ScopedLock.h"
#include "Poco/Net/SSLManager.h"

#include "EasyHttpContext.h"

namespace easyhttpcpp {

const unsigned int EasyHttpContext::DefaultTimeoutSec = 60;

EasyHttpContext::EasyHttpContext() : m_timeoutSec(DefaultTimeoutSec), m_crlCheckPolicy(CrlCheckPolicyNoCheck)
{
}

EasyHttpContext::~EasyHttpContext()
{
    m_callInterceptors.clear();
    m_networkInterceptors.clear();
}

void EasyHttpContext::setCache(HttpCache::Ptr pCache)
{
    m_pCache = pCache;
}

HttpCache::Ptr EasyHttpContext::getCache() const
{
    return m_pCache;
}

void EasyHttpContext::setTimeoutSec(unsigned int timeout)
{
    m_timeoutSec = timeout;
}

unsigned int EasyHttpContext::getTimeoutSec() const
{
    return m_timeoutSec;
}

void EasyHttpContext::setProxy(Proxy::Ptr pProxy)
{
    m_pProxy = pProxy;
}

Proxy::Ptr EasyHttpContext::getProxy() const
{
    return m_pProxy;
}

void EasyHttpContext::setRootCaDirectory(const std::string& rootCaDirectory)
{
    m_rootCaDirectory = rootCaDirectory;
}

const std::string& EasyHttpContext::getRootCaDirectory() const
{
    return m_rootCaDirectory;
}

void EasyHttpContext::setRootCaFile(const std::string& rootCaFile)
{
    m_rootCaFile = rootCaFile;
}

const std::string& EasyHttpContext::getRootCaFile() const
{
    return m_rootCaFile;
}

void EasyHttpContext::setCrlCheckPolicy(CrlCheckPolicy crlCheckPolicy)
{
    m_crlCheckPolicy = crlCheckPolicy;
}

CrlCheckPolicy EasyHttpContext::getCrlCheckPolicy() const
{
    return m_crlCheckPolicy;
}

void EasyHttpContext::setCallInterceptors(EasyHttpContext::InterceptorList& interceptors)
{
    m_callInterceptors = interceptors;
}

EasyHttpContext::InterceptorList& EasyHttpContext::getCallInterceptors()
{
    return m_callInterceptors;
}

void EasyHttpContext::setNetworkInterceptors(EasyHttpContext::InterceptorList& interceptors)
{
    m_networkInterceptors = interceptors;
}

EasyHttpContext::InterceptorList& EasyHttpContext::getNetworkInterceptors()
{
    return m_networkInterceptors;
}

void EasyHttpContext::setConnectionPool(ConnectionPool::Ptr pConnectionPool)
{
    m_pConnectionPool = pConnectionPool;
}

ConnectionPool::Ptr EasyHttpContext::getConnectionPool() const
{
    return m_pConnectionPool;
}

} /* namespace easyhttpcpp */
