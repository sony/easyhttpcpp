/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED
#define EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED

#include "easyhttpcpp/Call.h"
#include "easyhttpcpp/EasyHttp.h"

#include "EasyHttpContext.h"

namespace easyhttpcpp {

class EasyHttpInternal : public EasyHttp {
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

    EasyHttpContext::Ptr getHttpContenxt() const;
private:
    EasyHttpInternal();

    EasyHttpContext::Ptr m_pContext;

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EASYHTTPINTERNAL_H_INCLUDED */
