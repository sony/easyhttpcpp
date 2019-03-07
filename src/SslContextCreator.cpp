/*
 * Copyright 2017 Sony Corporation
 */

#include "SslContextCreator.h"
#include "SslContextCreatorImpl.h"

namespace easyhttpcpp {

Poco::Net::Context::Ptr SslContextCreator::createContext(EasyHttpContext::Ptr pContext)
{
    return SslContextCreatorImpl::createContext(pContext);
}

} /* namespace easyhttpcpp */
