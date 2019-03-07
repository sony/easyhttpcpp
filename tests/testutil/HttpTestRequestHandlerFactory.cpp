/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>

#include "Poco/URI.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "HttpTestRequestHandlerFactory.h"

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "HttpTestRequestHandlerFactory";
const std::string HttpTestRequestHandlerFactory::HeaderHandlerId = "X-Handler-Id";

HttpTestRequestHandlerFactory::HttpTestRequestHandlerFactory()
{
}

HttpTestRequestHandlerFactory::~HttpTestRequestHandlerFactory()
{
}

Poco::Net::HTTPRequestHandler* HttpTestRequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest& request)
{
    Poco::Net::HTTPRequestHandler* pTarget = NULL;
    Poco::URI uri(request.getURI());
    const std::string& handlerId = request.get(HeaderHandlerId, "");

    Poco::FastMutex::ScopedLock lock(m_handlerMutex);
    HandlerMap::Iterator it = m_handlers.find(makeKey(uri.getPath(), handlerId));
    if (it == m_handlers.end()) {
        EASYHTTPCPP_LOG_D(Tag, "not registered path [%s]", request.getURI().c_str());
    } else {
        EASYHTTPCPP_LOG_D(Tag, "request handler exist.[%s]", request.getURI().c_str());
        pTarget = it->second;
    }
    return new RequestHandlerWorker(pTarget);
}

void HttpTestRequestHandlerFactory::addHandler(const std::string& path,
        Poco::Net::HTTPRequestHandler* pHandler)
{
    addHandler(path, pHandler, "");
}

void HttpTestRequestHandlerFactory::addHandler(const std::string& path, Poco::Net::HTTPRequestHandler* pHandler,
        const std::string& handlerId)
{
    Poco::FastMutex::ScopedLock lock(m_handlerMutex);

    m_handlers[makeKey(path, handlerId)] = pHandler;
    EASYHTTPCPP_LOG_D(Tag, "addHandler : path=[%s] handlerId=[%s] m_handlers.size=%zu", path.c_str(), handlerId.c_str(),
            m_handlers.size());
}

void HttpTestRequestHandlerFactory::removeHandler(const std::string& path)
{
    removeHandler(path, "");
}

void HttpTestRequestHandlerFactory::removeHandler(const std::string& path, const std::string& handlerId)
{
    Poco::FastMutex::ScopedLock lock(m_handlerMutex);

    m_handlers.erase(makeKey(path, handlerId));
    EASYHTTPCPP_LOG_D(Tag, "removeHandler : path=[%s] handlerId=[%s] m_handlers.size=%zu", path.c_str(), handlerId.c_str(),
            m_handlers.size());
}

void HttpTestRequestHandlerFactory::clearAllHandler()
{
    Poco::FastMutex::ScopedLock lock(m_handlerMutex);

    m_handlers.clear();
}

HttpTestRequestHandlerFactory::RequestHandlerWorker::RequestHandlerWorker(Poco::Net::HTTPRequestHandler* pTarget) :
m_pTarget(pTarget)
{
}

void HttpTestRequestHandlerFactory::RequestHandlerWorker::handleRequest(Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response)
{
    if (m_pTarget != NULL) {
        m_pTarget->handleRequest(request, response);
    } else {
        EASYHTTPCPP_LOG_D(Tag, "Unhandled request was found.");

        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.setReason(Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);

        response.setContentType("text/html");
        std::ostream& ostr = response.send();
        ostr << "<html><head><title>TestHttpServer</title>";
        ostr << "<body>";
        ostr << Poco::Net::HTTPResponse::HTTP_NOT_FOUND << " " << Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND;
        ostr << "</p></body></html>";
    }
}

std::string HttpTestRequestHandlerFactory::makeKey(const std::string& path, const std::string& handlerId)
{
    std::string key;
    if (!handlerId.empty()) {
        key = path + "_" + handlerId;
    } else {
        key = path;
    }
    return key;
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
