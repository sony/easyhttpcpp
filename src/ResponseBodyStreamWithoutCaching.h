/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODYSTREAMWITHOURCACHING_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODYSTREAMWITHOURCACHING_H_INCLUDED

#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "HttpEngine.h"
#include "ResponseBodyStreamInternal.h"

namespace easyhttpcpp {

class ResponseBodyStreamWithoutCaching : public ResponseBodyStreamInternal {
public:
    ResponseBodyStreamWithoutCaching(std::istream& content, ConnectionInternal::Ptr pConnectionInternal,
            ConnectionPoolInternal::Ptr pConnectionPoolInternal);
    virtual ~ResponseBodyStreamWithoutCaching();
    virtual void close();

    ResponseBodyStream::Ptr exchangeToResponseBodyStreamWithCaching(Response::Ptr pResponse, HttpCache::Ptr pHttpCache);
    Connection::Ptr getConnection();    // for test

private:
    ConnectionInternal::Ptr m_pConnectionInternal;
    ConnectionPoolInternal::Ptr m_pConnectionPoolInternal;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODYSTREAMWITHOURCACHING_H_INCLUDED */
