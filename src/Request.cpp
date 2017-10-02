/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "Poco/URI.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/common/StringUtil.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "Request";

Request::Request(Request::Builder& builder)
{
    initFromBuilder(builder);
}

Request::Request(Request::Ptr pRequest)
{
    Request::Builder builder(pRequest);
    initFromBuilder(builder);
}

Request::~Request()
{
}

RequestBody::Ptr Request::getBody() const
{
    return m_pBody;
}

CacheControl::Ptr Request::getCacheControl() const
{
    return m_pCacheControl;
}

const std::string& Request::getHeaderValue(const std::string& name, const std::string& defaultValue) const
{
    if (!m_pHeaders) {
        return defaultValue;
    } else {
        return m_pHeaders->getValue(name, defaultValue);
    }
}

bool Request::hasHeader(const std::string& name) const
{
    if (!m_pHeaders) {
        return false;
    } else {
        return m_pHeaders->has(name);
    }
}

Headers::Ptr Request::getHeaders() const
{
    return m_pHeaders;
}

Request::HttpMethod Request::getMethod() const
{
    return m_method;
}

const void* Request::getTag() const
{
    return m_pTag;
}

const std::string& Request::getUrl() const
{
    return m_url;
}

void Request::initFromBuilder(Builder& builder)
{
    m_method = builder.getMethod();
    m_url = builder.getUrl();
    m_pTag = builder.getTag();
    if (builder.getHeaders()) {
        m_pHeaders = new Headers(*(builder.getHeaders()));
    } else {
        m_pHeaders = new Headers();
    }
    m_pCacheControl = builder.getCacheControl();
    if (!m_pCacheControl) {
        m_pCacheControl = CacheControl::createFromHeaders(m_pHeaders);
    }
    m_pBody = builder.getBody();
}

Request::Builder::Builder() : m_method(HttpMethodGet), m_pTag(NULL)
{
}

Request::Builder::Builder(Request::Ptr pRequest)
{
    if (!pRequest) {
        std::string message = "can not create Request::Builder because Request is NULL.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }

    m_method = pRequest->m_method;
    m_url = pRequest->m_url;
    m_pTag = pRequest->m_pTag;
    m_pHeaders = pRequest->m_pHeaders;
    m_pCacheControl = pRequest->m_pCacheControl;
    m_pBody = pRequest->m_pBody;
}

Request::Builder::~Builder()
{
}

Request::Ptr Request::Builder::build()
{
    return new Request(*this);
}

Request::Builder& Request::Builder::httpDelete()
{
    m_method = HttpMethodDelete;
    return *this;
}

Request::Builder& Request::Builder::httpDelete(RequestBody::Ptr pBody)
{
    m_method = HttpMethodDelete;
    m_pBody = pBody;
    return *this;
}

Request::Builder& Request::Builder::httpGet()
{
    m_method = HttpMethodGet;
    return *this;
}

Request::Builder& Request::Builder::httpHead()
{
    m_method = HttpMethodHead;
    return *this;
}

Request::Builder& Request::Builder::httpPatch(RequestBody::Ptr pBody)
{
    m_method = HttpMethodPatch;
    m_pBody = pBody;
    return *this;
}

Request::Builder& Request::Builder::httpPost(RequestBody::Ptr pBody)
{
    m_method = HttpMethodPost;
    m_pBody = pBody;
    return *this;
}

Request::Builder& Request::Builder::httpPost()
{
    m_method = HttpMethodPost;
    return *this;
}

Request::Builder& Request::Builder::httpPut(RequestBody::Ptr pBody)
{
    m_method = HttpMethodPut;
    m_pBody = pBody;
    return *this;
}

Request::HttpMethod Request::Builder::getMethod() const
{
    return m_method;
}

RequestBody::Ptr Request::Builder::getBody() const
{
    return m_pBody;
}

Request::Builder& Request::Builder::setCacheControl(CacheControl::Ptr pCacheControl)
{
    m_pCacheControl = pCacheControl;
    return *this;
}

CacheControl::Ptr Request::Builder::getCacheControl() const
{
    return m_pCacheControl;
}

Request::Builder& Request::Builder::setHeader(const std::string& name, const std::string& value)
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

Request::Builder& Request::Builder::setHeaders(Headers::Ptr pHeaders)
{
    m_pHeaders = pHeaders;
    return *this;
}
        
Headers::Ptr Request::Builder::getHeaders() const
{
    return m_pHeaders;
}

Request::Builder& Request::Builder::setTag(const void* pTag)
{
    m_pTag = pTag;
    return *this;
}

const void* Request::Builder::getTag() const
{
    return m_pTag;
}

Request::Builder& Request::Builder::setUrl(const std::string& url)
{
    if (url.empty()) {
        std::string message = "can not set empty url.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    // check that can be encoded,
    try {
        Poco::URI uri(url);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "setUrl: invalid url[%s] message=%s", url.c_str(), e.message().c_str());
        throw HttpIllegalArgumentException(StringUtil::format("[%s] is invalid url. extra message=%s", url.c_str(),
                e.message().c_str()), e);
    }
    m_url = url;
    return *this;
}

const std::string& Request::Builder::getUrl() const
{
    return m_url;
}

} /* namespace easyhttpcpp */
