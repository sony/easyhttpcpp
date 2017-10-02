/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

#include "ResponseBodyStreamWithoutCaching.h"
#include "ResponseBodyStreamWithCaching.h"

#include "ConnectionPoolInternal.h"

namespace easyhttpcpp {

static const std::string Tag = "ResponseBodyStreamWithoutCaching";

ResponseBodyStreamWithoutCaching::ResponseBodyStreamWithoutCaching(std::istream& content,
        ConnectionInternal::Ptr pConnectionInternal, ConnectionPoolInternal::Ptr pConnectionPoolInternal)
        : ResponseBodyStreamInternal(content), m_pConnectionInternal(pConnectionInternal),
        m_pConnectionPoolInternal(pConnectionPoolInternal)
{
}

ResponseBodyStreamWithoutCaching::~ResponseBodyStreamWithoutCaching()
{
}

void ResponseBodyStreamWithoutCaching::close()
{
    Poco::Mutex::ScopedLock lock(m_instanceMutex);

    if (m_closed) {
        EASYHTTPCPP_LOG_D(Tag, "close: already closed");
        return;
    }

    // after exchangeToResponseBodyStreamWithCaching, m_pConnectionInternal is NULL.
    if (m_pConnectionInternal) {

        // skip remaining data, if exist.
        if (skipAll(m_pConnectionInternal->getPocoHttpClientSession())) {
            EASYHTTPCPP_LOG_D(Tag, "Connection is reusable.");
            // release connection.
            m_pConnectionPoolInternal->releaseConnection(m_pConnectionInternal);
        } else {
            EASYHTTPCPP_LOG_D(Tag, "Could not reuse connection since skipping response body failed.");
            // remove connection.
            m_pConnectionPoolInternal->removeConnection(m_pConnectionInternal);
        }
        m_pConnectionInternal = NULL;
    }
    m_pConnectionPoolInternal = NULL;

    ResponseBodyStreamInternal::close();
    EASYHTTPCPP_LOG_D(Tag, "close: finished");
}

Connection::Ptr ResponseBodyStreamWithoutCaching::getConnection()
{
    return m_pConnectionInternal.unsafeCast<Connection>();
}

ResponseBodyStream::Ptr ResponseBodyStreamWithoutCaching::exchangeToResponseBodyStreamWithCaching(
        Response::Ptr pResponse, HttpCache::Ptr pHttpCache)
{
    ResponseBodyStream::Ptr pNewResponseBodyStream = new ResponseBodyStreamWithCaching(
            m_content, m_pConnectionInternal, m_pConnectionPoolInternal, pResponse, pHttpCache);

    // to close state for do not touch stream
    m_pConnectionInternal = NULL;
    ResponseBodyStreamInternal::close();

    return pNewResponseBodyStream;
}

} /* namespace easyhttpcpp */
