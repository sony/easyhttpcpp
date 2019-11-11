/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED
#define EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED

#include "Poco/Mutex.h"

#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpExports.h"

#include "EasyHttpContext.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API EasyHttpInternal : public EasyHttp {
public:
    EasyHttpInternal(EasyHttp::Builder& builder);
    virtual ~EasyHttpInternal();
    virtual Call::Ptr newCall(Request::Ptr pRequest);
    virtual Proxy::Ptr getProxy() const;
    virtual unsigned int getTimeoutSec() const;
    virtual HttpCache::Ptr getCache() const;
    virtual CrlCheckPolicy getCrlCheckPolicy() const;
    virtual const std::string& getRootCaDirectory() const;
    virtual const std::string& getRootCaFile() const;
    virtual ConnectionPool::Ptr getConnectionPool() const;
    virtual unsigned int getCorePoolSizeOfAsyncThreadPool() const;
    virtual unsigned int getMaximumPoolSizeOfAsyncThreadPool() const;
    virtual void invalidateAndCancel();

    EasyHttpContext::Ptr getHttpContenxt() const;
private:
    EasyHttpInternal();

    EasyHttpContext::Ptr m_pContext;
    unsigned int m_corePoolSizeOfAsyncThreadPool;
    unsigned int m_maximumPoolSizeOfAsyncThreadPool;
    Poco::FastMutex m_instanceMutex;
    bool m_invalidated;

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED */
