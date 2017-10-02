/*
 * Copyright 2017 Sony Corporation
 */

#include "HttpCacheMetadata.h"

namespace easyhttpcpp {

HttpCacheMetadata::HttpCacheMetadata() : m_httpMethod(Request::HttpMethodGet), m_statusCode(-1), m_responseBodySize(0),
        m_sentRequestAtEpoch(0), m_receivedResponseAtEpoch(0), m_createdAtEpoch(0)

{
}

HttpCacheMetadata::~HttpCacheMetadata()
{
}

void HttpCacheMetadata::setUrl(const std::string& url)
{
    m_url = url;
}

const std::string& HttpCacheMetadata::getUrl() const
{
    return m_url;
}

void HttpCacheMetadata::setHttpMethod(Request::HttpMethod httpMethod)
{
    m_httpMethod = httpMethod;
}

Request::HttpMethod HttpCacheMetadata::getHttpMethod() const
{
    return m_httpMethod;
}

void HttpCacheMetadata::setStatusCode(int statusCode)
{
    m_statusCode = statusCode;
}

int HttpCacheMetadata::getStatusCode() const
{
    return m_statusCode;
}

void HttpCacheMetadata::setStatusMessage(const std::string& statusMessage)
{
    m_statusMessage = statusMessage;
}

const std::string& HttpCacheMetadata::getStatusMessage() const
{
    return m_statusMessage;
}

void HttpCacheMetadata::setResponseHeaders(Headers::Ptr pResponseHeaders)
{
    m_pResponseHeaders = pResponseHeaders;
}

Headers::Ptr HttpCacheMetadata::getResponseHeaders() const
{
    return m_pResponseHeaders;
}

void HttpCacheMetadata::setResponseBodySize(size_t responseBodySize)
{
    m_responseBodySize = responseBodySize;
}

size_t HttpCacheMetadata::getResponseBodySize() const
{
    return m_responseBodySize;
}

void HttpCacheMetadata::setSentRequestAtEpoch(std::time_t sentRequestAtEpoch)
{
    m_sentRequestAtEpoch = sentRequestAtEpoch;
}

std::time_t HttpCacheMetadata::getSentRequestAtEpoch() const
{
    return m_sentRequestAtEpoch;
}

void HttpCacheMetadata::setReceivedResponseAtEpoch(std::time_t receivedResponseAtEpoch)
{
    m_receivedResponseAtEpoch = receivedResponseAtEpoch;
}

std::time_t HttpCacheMetadata::getReceivedResponseAtEpoch() const
{
    return m_receivedResponseAtEpoch;
}

void HttpCacheMetadata::setCreatedAtEpoch(std::time_t createdAtEpoch)
{
    m_createdAtEpoch = createdAtEpoch;
}

std::time_t HttpCacheMetadata::getCreatedAtEpoch() const
{
    return m_createdAtEpoch;
}

} /* namespace easyhttpcpp */


