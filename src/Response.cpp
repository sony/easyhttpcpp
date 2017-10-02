/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Response.h"

namespace easyhttpcpp {

static const std::string Tag = "Response";

Response::Response(Response::Builder& builder)
{
    initFromBuilder(builder);
}

Response::Response(Response::Ptr pResponse)
{
    Response::Builder builder(pResponse);
    initFromBuilder(builder);
}

Response::~Response()
{
}

ResponseBody::Ptr Response::getBody() const
{
    return m_pBody;
}

CacheControl::Ptr Response::getCacheControl()
{
    return m_pCacheControl;
}

int Response::getCode() const
{
    return m_statusCode;
}

const std::string& Response::getMessage() const
{
    return m_statusMessage;
}

bool Response::hasContentLength() const
{
    return m_hasContentLength;
}

ssize_t Response::getContentLength() const
{
    return m_contentLength;
}

const std::string& Response::getHeaderValue(const std::string& name, const std::string& defaultValue) const
{
    if (!m_pHeaders) {
        return defaultValue;
    } else {
        return m_pHeaders->getValue(name, defaultValue);
    }
}

bool Response::hasHeader(const std::string& name) const
{
    if (!m_pHeaders) {
        return false;
    } else {
        return m_pHeaders->has(name);
    }
}

Headers::Ptr Response::getHeaders() const
{
    return m_pHeaders;
}

Request::Ptr Response::getRequest() const
{
    return m_pRequest;
}

Response::Ptr Response::getCacheResponse() const
{
    return m_pCacheResponse;
}

Response::Ptr Response::getNetworkResponse() const
{
    return m_pNetworkResponse;
}

Response::Ptr Response::getPriorResponse() const
{
    return m_pPriorResponse;
}

std::time_t Response::getSentRequestSec() const
{
    return m_sentRequestSec;
}

std::time_t Response::getReceivedResponseSec() const
{
    return m_receivedResponseSec;
}

bool Response::isSuccessful() const
{
    return m_statusCode >= Poco::Net::HTTPResponse::HTTP_OK &&
            m_statusCode < Poco::Net::HTTPResponse::HTTP_MULTIPLE_CHOICES;
}

std::string Response::toString()
{
    // TODO: later
    return "";
}

void Response::initFromBuilder(Response::Builder& builder)
{
    m_statusCode = builder.getCode();
    m_statusMessage = builder.getMessage();
    m_hasContentLength = builder.hasContentLength();
    m_contentLength = builder.getContentLength();
    if (builder.getHeaders()) {
        m_pHeaders = new Headers(*(builder.getHeaders()));
    } else {
        m_pHeaders = new Headers();
    }
    m_pCacheControl = builder.getCacheControl();
    m_pBody = builder.getBody();
    m_pRequest = builder.getRequest();
    m_pCacheResponse = builder.getCacheResponse();
    m_pNetworkResponse = builder.getNetworkResponse();
    m_pPriorResponse = builder.getPriorResponse();
    m_sentRequestSec = builder.getSentRequestSec();
    m_receivedResponseSec = builder.getReceivedResponseSec();
}

Response::Builder::Builder() : m_statusCode(200), m_hasContentLength(false), m_contentLength(-1),
        m_sentRequestSec(0), m_receivedResponseSec(0)
{
}

Response::Builder::Builder(Response::Ptr pResponse)
{
    if (!pResponse) {
        std::string message = "can not create Response::Builder because Response is NULL.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }

    m_statusCode = pResponse->m_statusCode;
    m_statusMessage = pResponse->m_statusMessage;
    m_hasContentLength = pResponse->m_hasContentLength;
    m_contentLength = pResponse->m_contentLength;
    m_pHeaders = pResponse->m_pHeaders;
    m_pCacheControl = pResponse->m_pCacheControl;
    m_pBody = pResponse->m_pBody;
    m_pRequest = pResponse->m_pRequest;
    m_pCacheResponse = pResponse->m_pCacheResponse;
    m_pNetworkResponse = pResponse->m_pNetworkResponse;
    m_pPriorResponse = pResponse->m_pPriorResponse;
    m_sentRequestSec = pResponse->getSentRequestSec();
    m_receivedResponseSec = pResponse->getReceivedResponseSec();
}

Response::Builder::~Builder()
{
}

Response::Ptr Response::Builder::build()
{
    return new Response(*this);
}

Response::Builder& Response::Builder::setBody(ResponseBody::Ptr pBody)
{
    m_pBody = pBody;
    return *this;
}

ResponseBody::Ptr Response::Builder::getBody() const
{
    return m_pBody;
}

Response::Builder& Response::Builder::setCode(int code)
{
    m_statusCode = code;
    return *this;
}

int Response::Builder::getCode() const
{
    return m_statusCode;
}

Response::Builder& Response::Builder::setMessage(const std::string& message)
{
    m_statusMessage = message;
    return *this;
}

const std::string& Response::Builder::getMessage() const
{
    return m_statusMessage;
}

Response::Builder& Response::Builder::setHasContentLength(bool hasContentLength)
{
    m_hasContentLength = hasContentLength;
    return *this;
}

bool Response::Builder::hasContentLength() const
{
    if (m_pBody) {
        return m_pBody->hasContentLength();
    }
    return m_hasContentLength;
}

Response::Builder& Response::Builder::setContentLength(ssize_t contentLength)
{
    m_contentLength = contentLength;
    return *this;
}

ssize_t Response::Builder::getContentLength() const
{
    if (m_pBody) {
        return m_pBody->getContentLength();
    }
    return m_contentLength;
}

Response::Builder& Response::Builder::setCacheControl(CacheControl::Ptr pCacheControl)
{
    m_pCacheControl = pCacheControl;
    return *this;
}

CacheControl::Ptr Response::Builder::getCacheControl() const
{
    return m_pCacheControl;
}

Response::Builder& Response::Builder::setHeader(const std::string& name, const std::string& value)
{
    if (name.empty()) {
        std::string message = "can not set empty name in header.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    if (!m_pHeaders) {
        m_pHeaders = new Headers();
    }
    m_pHeaders->set(name, value);
    return *this;
}

Response::Builder& Response::Builder::addHeader(const std::string& name, const std::string& value)
{
    if (name.empty()) {
        std::string message = "can not set empty name in header.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    if (!m_pHeaders) {
        m_pHeaders = new Headers();
    }
    m_pHeaders->add(name, value);
    return *this;
}

Response::Builder& Response::Builder::setHeaders(Headers::Ptr pHeaders)
{
    m_pHeaders = pHeaders;
    return *this;
}

Headers::Ptr Response::Builder::getHeaders() const
{
    return m_pHeaders;
}

Response::Builder& Response::Builder::setRequest(Request::Ptr pRequest)
{
    m_pRequest = pRequest;
    return *this;
}

Request::Ptr Response::Builder::getRequest()
{
    return m_pRequest;
}

Response::Builder& Response::Builder::setCacheResponse(Response::Ptr pResponse)
{
    m_pCacheResponse = pResponse;
    return *this;
}

Response::Ptr Response::Builder::getCacheResponse() const
{
    return m_pCacheResponse;
}

Response::Builder& Response::Builder::setNetworkResponse(Response::Ptr pResponse)
{
    m_pNetworkResponse = pResponse;
    return *this;
}

Response::Ptr Response::Builder::getNetworkResponse() const
{
    return m_pNetworkResponse;
}

Response::Builder& Response::Builder::setPriorResponse(Response::Ptr pResponse)
{
    m_pPriorResponse = pResponse;
    return *this;
}

Response::Ptr Response::Builder::getPriorResponse() const
{
    return m_pPriorResponse;
}

Response::Builder& Response::Builder::setSentRequestSec(std::time_t sentRequestSec)
{
    m_sentRequestSec = sentRequestSec;
    return *this;
}

std::time_t Response::Builder::getSentRequestSec() const
{
    return m_sentRequestSec;
}

Response::Builder& Response::Builder::setReceivedResponseSec(std::time_t receivedResponseSec)
{
    m_receivedResponseSec = receivedResponseSec;
    return *this;
}

std::time_t Response::Builder::getReceivedResponseSec() const
{
    return m_receivedResponseSec;
}

} /* namespace easyhttpcpp */

