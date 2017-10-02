/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/NumberFormatter.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"

#include "CallInternal.h"
#include "EasyHttpContext.h"
#include "EasyHttpInternal.h"

namespace easyhttpcpp {

static const std::string Tag = "EasyHttp::Builder";

EasyHttp::EasyHttp()
{
}

EasyHttp::~EasyHttp()
{
}

EasyHttp::Builder::Builder() : m_timeoutSec(EasyHttpContext::DefaultTimeoutSec),
        m_crlCheckPolicy(CrlCheckPolicyNoCheck)
{
}

EasyHttp::Builder::~Builder()
{
    m_callInterceptors.clear();
    m_networkInterceptors.clear();
}

EasyHttp::Ptr EasyHttp::Builder::build()
{
    return new EasyHttpInternal(*this);
}

EasyHttp::Builder& EasyHttp::Builder::setCache(HttpCache::Ptr pCache)
{
    m_pCache = pCache;
    return *this;
}

HttpCache::Ptr EasyHttp::Builder::getCache() const
{
    return m_pCache;
}

EasyHttp::Builder& EasyHttp::Builder::setTimeoutSec(unsigned int timeout)
{
    if (timeout == 0) {
        std::string message = std::string("can not set 0 to timeout.[") +
                Poco::NumberFormatter::format(timeout) + "]";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    m_timeoutSec = timeout;
    return *this;
}

unsigned int EasyHttp::Builder::getTimeoutSec() const
{
    return m_timeoutSec;
}

EasyHttp::Builder& EasyHttp::Builder::setProxy(Proxy::Ptr pProxy)
{
    m_pProxy = pProxy;
    return *this;
}

Proxy::Ptr EasyHttp::Builder::getProxy() const
{
    return m_pProxy;
}

EasyHttp::Builder& EasyHttp::Builder::setRootCaDirectory(const std::string& rootCaDirectory)
{
    if (rootCaDirectory.empty()) {
        std::string message = "can not set empty root CA directory.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    m_rootCaDirectory = rootCaDirectory;
    return *this;
}

const std::string& EasyHttp::Builder::getRootCaDirectory() const
{
    return m_rootCaDirectory;
}

EasyHttp::Builder& EasyHttp::Builder::setRootCaFile(const std::string& rootCaFile)
{
    if (rootCaFile.empty()) {
        std::string message = "can not set empty root CA file.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    m_rootCaFile = rootCaFile;
    return *this;
}

const std::string& EasyHttp::Builder::getRootCaFile() const
{
    return m_rootCaFile;
}

EasyHttp::Builder& EasyHttp::Builder::setCrlCheckPolicy(CrlCheckPolicy crlCheckPolicy)
{
    m_crlCheckPolicy = crlCheckPolicy;
    return *this;
}

CrlCheckPolicy EasyHttp::Builder::getCrlCheckPolicy() const
{
    return m_crlCheckPolicy;
}

EasyHttp::Builder& EasyHttp::Builder::addInterceptor(Interceptor::Ptr pInterceptor)
{
    m_callInterceptors.push_back(pInterceptor);
    return *this;
}

EasyHttpContext::InterceptorList& EasyHttp::Builder::getInterceptors()
{
    return m_callInterceptors;
}

EasyHttp::Builder& EasyHttp::Builder::addNetworkInterceptor(Interceptor::Ptr pInterceptor)
{
    m_networkInterceptors.push_back(pInterceptor);
    return *this;
}

EasyHttpContext::InterceptorList& EasyHttp::Builder::getNetworkInterceptors()
{
    return m_networkInterceptors;
}

EasyHttp::Builder& EasyHttp::Builder::setConnectionPool(ConnectionPool::Ptr pConnectionPool)
{
    m_pConnectionPool = pConnectionPool;
    return *this;
}

ConnectionPool::Ptr EasyHttp::Builder::getConnectionPool() const
{
    return m_pConnectionPool;
}

} /* namespace easyhttpcpp */
