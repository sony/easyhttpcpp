/*
 * Copyright 2017 Sony Corporation
 */

#include <ostream>

#include "Poco/Net/HTTPResponse.h"
#include "Poco/NumberParser.h"
#include "Poco/Thread.h"
#include "Poco/URI.h"

#include "HttpTestConstants.h"
#include "HttpTestCommonRequestHandler.h"

namespace easyhttpcpp {
namespace test {

static const int TestFailureTimeout = 10 * 1000; // milliseconds
static const char* const HeaderConnection = "Connection";

void HttpTestCommonRequestHandler::OkRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    saveRequestParameterAsString(request);

    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

void HttpTestCommonRequestHandler::OneHourMaxAgeRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
    response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

void HttpTestCommonRequestHandler::BadRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    saveRequestParameterAsString(request);

    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    response.setContentLength(strlen(HttpTestConstants::BadRequestResponseBody));

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::BadRequestResponseBody;
}

void HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler1st::handleRequest(
        Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    response.set(HttpTestConstants::HeaderLastModified, HttpTestConstants::HeaderValueLastModified);
    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

void HttpTestCommonRequestHandler::NotModifiedResponseRequestHandler2nd::handleRequest(
        Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
    response.send();
}

HttpTestCommonRequestHandler::WaitRequestHandler::~WaitRequestHandler()
{
    set();
}

void HttpTestCommonRequestHandler::WaitRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    saveRequestParamemter(request);

    m_waitEvent.tryWait(TestFailureTimeout);

    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

void HttpTestCommonRequestHandler::WaitRequestHandler::set()
{
    m_waitEvent.set();
}

void HttpTestCommonRequestHandler::ConnectionCloseRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    saveRequestParameterAsString(request);

    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
    response.setKeepAlive(false);

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

void HttpTestCommonRequestHandler::NoConnectionRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    saveRequestParameterAsString(request);

    response.setContentType(HttpTestConstants::DefaultResponseContentType);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentLength(strlen(HttpTestConstants::DefaultResponseBody));
    response.erase(HeaderConnection);

    std::ostream& ostr = response.send();
    ostr << HttpTestConstants::DefaultResponseBody;
}

HttpTestCommonRequestHandler::SpecifyingContentLengthRequestHandler::SpecifyingContentLengthRequestHandler(
        ssize_t contentLength) : m_contentLength(contentLength)
{
}

void HttpTestCommonRequestHandler::SpecifyingContentLengthRequestHandler::handleRequest(
        Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    Poco::Buffer<char> responseBody(m_contentLength);
    char* pBuffer = responseBody.begin();
    for (ssize_t i = 0; i < m_contentLength; i++) {
        pBuffer[i] = static_cast<char> (i & 0xff);
    }
    response.setContentLength(m_contentLength);
    std::ostream& ostr = response.send();
    ostr.write(pBuffer, m_contentLength);
}

HttpTestCommonRequestHandler::WaitInTheMiddleOfWriteResponseBodyRequestHandler::
WaitInTheMiddleOfWriteResponseBodyRequestHandler(ssize_t contentLength, ssize_t writeBytesBeforeSleep,
        long sleepMilliSec) : m_contentLength(contentLength), m_writeBytesBeforeSleep(writeBytesBeforeSleep),
        m_sleepMilliSec(sleepMilliSec), m_needToWaitHandler(false)
{
}

HttpTestCommonRequestHandler::WaitInTheMiddleOfWriteResponseBodyRequestHandler::
~WaitInTheMiddleOfWriteResponseBodyRequestHandler()
{
    if (m_needToWaitHandler) {
        m_finishEvent.tryWait(TestFailureTimeout);
    }
}

void HttpTestCommonRequestHandler::WaitInTheMiddleOfWriteResponseBodyRequestHandler::handleRequest(
        Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    response.set(HttpTestConstants::HeaderCacheControl, HttpTestConstants::MaxAgeOneHour);
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    Poco::Buffer<char> responseBody(m_contentLength);
    char* pBuffer = responseBody.begin();
    for (ssize_t i = 0; i < m_contentLength; i++) {
        pBuffer[i] = static_cast<char> (i & 0xff);
    }
    response.setContentLength(m_contentLength);
    std::ostream& ostr = response.send();
    ostr.write(pBuffer, m_writeBytesBeforeSleep);
    ostr.flush();
    m_needToWaitHandler = true;
    Poco::Thread::sleep(m_sleepMilliSec);
    ostr.write(pBuffer + m_writeBytesBeforeSleep, m_contentLength - m_writeBytesBeforeSleep);
    m_finishEvent.set();
}

} /* namespace test */
} /* namespace easyhttpcpp */
