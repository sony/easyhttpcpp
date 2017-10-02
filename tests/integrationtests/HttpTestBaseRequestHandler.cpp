/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>

#include "Poco/StreamCopier.h"

#include "easyhttpcpp/common/CoreLogger.h"

#include "HttpTestBaseRequestHandler.h"

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "HttpTestBaseRequestHandler";
static const size_t RequestBufferBytes = 8192;

HttpTestBaseRequestHandler::HttpTestBaseRequestHandler() : m_maxRequestBodyBufferSize(RequestBufferBytes)
{
    clearParameter();
}

HttpTestBaseRequestHandler::~HttpTestBaseRequestHandler()
{
}

void HttpTestBaseRequestHandler::saveRequestParamemter(Poco::Net::HTTPServerRequest& request)
{
    m_method = request.getMethod();
    m_uri = request.getURI();
    m_requestHeaders = request;
    EASYHTTPCPP_LOG_D(Tag, "request.size=%d", request.size());
    for (Poco::Net::NameValueCollection::ConstIterator it = request.begin(); it != request.end(); it++) {
        EASYHTTPCPP_LOG_D(Tag, "%s %s", it->first.c_str(), it->second.c_str());
    }
    m_requestContentType = request.getContentType();
    m_hasRequestContentLength = request.hasContentLength();
    if (m_hasRequestContentLength) {
        m_requestContentLength = request.getContentLength();
    }
}

void HttpTestBaseRequestHandler::saveRequestParameterAsString(Poco::Net::HTTPServerRequest& request)
{
    saveRequestParamemter(request);

    std::ostringstream requestStream;
    std::istream& requestBodyStream = request.stream();
    Poco::StreamCopier::copyToString(requestBodyStream, m_requestBody);
}

void HttpTestBaseRequestHandler::saveRequestParameterAsBinary(Poco::Net::HTTPServerRequest& request)
{
    saveRequestParamemter(request);

    std::ostringstream requestStream;
    std::istream& requestBodyStream = request.stream();
    m_pRequestBodyBuffer = new Poco::Buffer<char>(m_maxRequestBodyBufferSize);
    m_requestBodyBytes = 0;
    while (!requestBodyStream.eof()) {
        requestBodyStream.read(m_pRequestBodyBuffer->begin() + m_requestBodyBytes,
                m_maxRequestBodyBufferSize - m_requestBodyBytes);
        if (requestBodyStream || requestBodyStream.eof()) {
            m_requestBodyBytes += requestBodyStream.gcount();
        }
    }
}

void HttpTestBaseRequestHandler::clearParameter()
{
    m_method.clear();
    m_hasRequestContentLength = false;
    m_requestContentLength = 0;
    m_requestBodyBytes = -1;
    m_requestBody.clear();
    m_pRequestBodyBuffer = NULL;
    m_requestContentType.clear();
    m_uri.clear();
    m_requestHeaders.clear();
}

const std::string& HttpTestBaseRequestHandler::getMethod() const
{
    return m_method;
}

const Poco::URI& HttpTestBaseRequestHandler::getUri() const
{
    return m_uri;
}

const Poco::Net::NameValueCollection& HttpTestBaseRequestHandler::getRequestHeaders() const
{
    return m_requestHeaders;
}

const std::string& HttpTestBaseRequestHandler::getRequestContentType() const
{
    return m_requestContentType;
}

const std::string& HttpTestBaseRequestHandler::getRequestBody() const
{
    return m_requestBody;
}

ssize_t HttpTestBaseRequestHandler::getRequestContentLength() const
{
    return m_requestContentLength;
}

size_t HttpTestBaseRequestHandler::getRequestBodyBytes() const
{
    return m_requestBodyBytes;
}

Poco::SharedPtr< Poco::Buffer<char> > HttpTestBaseRequestHandler::getRequestBodyBuffer() const
{
    return m_pRequestBodyBuffer;
}

void HttpTestBaseRequestHandler::setMaxRequestBodyBufferSize(size_t maxRequestBodyBufferSize)
{
    m_maxRequestBodyBufferSize = maxRequestBodyBufferSize;
}

} /* namespace test */
} /* namespace easyhttpcpp */
