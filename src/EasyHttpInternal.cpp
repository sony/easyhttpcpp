/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

#include "CallInternal.h"
#include "EasyHttpInternal.h"

namespace easyhttpcpp {

static const std::string Tag = "EasyHttpInternal";

EasyHttpInternal::EasyHttpInternal(EasyHttp::Builder& builder) : m_invalidated(false)
{
    m_pContext = new EasyHttpContext();
    m_pContext->setCache(builder.getCache());
    m_pContext->setTimeoutSec(builder.getTimeoutSec());
    m_pContext->setProxy(builder.getProxy());
    m_pContext->setRootCaDirectory(builder.getRootCaDirectory());
    m_pContext->setRootCaFile(builder.getRootCaFile());
    m_pContext->setCrlCheckPolicy(builder.getCrlCheckPolicy());
    m_pContext->setCallInterceptors(builder.getInterceptors());
    m_pContext->setNetworkInterceptors(builder.getNetworkInterceptors());
    m_pContext->setConnectionPool(builder.getConnectionPool());
    m_corePoolSizeOfAsyncThreadPool = builder.getCorePoolSizeOfAsyncThreadPool();
    m_maximumPoolSizeOfAsyncThreadPool = builder.getMaximumPoolSizeOfAsyncThreadPool();
    m_pContext->setHttpExecutionTaskManager(new HttpExecutionTaskManager(m_corePoolSizeOfAsyncThreadPool,
            m_maximumPoolSizeOfAsyncThreadPool));
}

EasyHttpInternal::~EasyHttpInternal()
{
    m_pContext->getHttpExecutionTaskManager()->gracefulShutdown();
}

Call::Ptr EasyHttpInternal::newCall(Request::Ptr pRequest)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);
    if (m_invalidated) {
        EASYHTTPCPP_LOG_D(Tag, "newCall: EasyHttp is already invalidated.");
        throw HttpIllegalStateException("Need to instantiate EasyHttp object again before calling newCall.");
    }

    if (!pRequest) {
        EASYHTTPCPP_LOG_D(Tag, "newCall: pRequest is NULL.");
        throw HttpIllegalArgumentException("newCall require Request.");
    }
    return new CallInternal(m_pContext, pRequest);
}

Proxy::Ptr EasyHttpInternal::getProxy() const
{
    return m_pContext->getProxy();
}

unsigned int EasyHttpInternal::getTimeoutSec() const
{
    return m_pContext->getTimeoutSec();
}

HttpCache::Ptr EasyHttpInternal::getCache() const
{
    return m_pContext->getCache();
}

CrlCheckPolicy EasyHttpInternal::getCrlCheckPolicy() const
{
    return m_pContext->getCrlCheckPolicy();
}

const std::string& EasyHttpInternal::getRootCaDirectory() const
{
    return m_pContext->getRootCaDirectory();
}

const std::string& EasyHttpInternal::getRootCaFile() const
{
    return m_pContext->getRootCaFile();
}

ConnectionPool::Ptr EasyHttpInternal::getConnectionPool() const
{
    return m_pContext->getConnectionPool();
}

EasyHttpContext::Ptr EasyHttpInternal::getHttpContenxt() const
{
    return m_pContext;
}

unsigned int EasyHttpInternal::getCorePoolSizeOfAsyncThreadPool() const
{
    return m_corePoolSizeOfAsyncThreadPool;
}

unsigned int EasyHttpInternal::getMaximumPoolSizeOfAsyncThreadPool() const
{
    return m_maximumPoolSizeOfAsyncThreadPool;
}

void EasyHttpInternal::invalidateAndCancel()
{
    {
        Poco::FastMutex::ScopedLock lock(m_instanceMutex);
        if (m_invalidated) {
            EASYHTTPCPP_LOG_D(Tag, "EasyHttp is already invalidated.");
            return;
        }
        m_invalidated = true;
    }

    m_pContext->getHttpExecutionTaskManager()->gracefulShutdown();
}

} /* namespace easyhttpcpp */
