/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_HTTPTESTREQUESTHANDLERFACTORY_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_HTTPTESTREQUESTHANDLERFACTORY_H_INCLUDED

#include <string>

#include "Poco/HashMap.h"
#include "Poco/Mutex.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"

#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

class EASYHTTPCPP_TESTUTIL_API HttpTestRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    HttpTestRequestHandlerFactory();
    virtual ~HttpTestRequestHandlerFactory();
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);
    void addHandler(const std::string& path, Poco::Net::HTTPRequestHandler* pHandler);
    void addHandler(const std::string& path, Poco::Net::HTTPRequestHandler* pHandler, const std::string& handlerId);
    void removeHandler(const std::string& path);
    void removeHandler(const std::string& path, const std::string& handlerId);
    void clearAllHandler();

    static const std::string HeaderHandlerId;

private:
    std::string makeKey(const std::string& path, const std::string& handlerId);

    typedef Poco::HashMap<std::string, Poco::Net::HTTPRequestHandler*> HandlerMap;
    HandlerMap m_handlers;
    Poco::FastMutex m_handlerMutex;

private:

    class RequestHandlerWorker : public Poco::Net::HTTPRequestHandler {
    public:
        RequestHandlerWorker(Poco::Net::HTTPRequestHandler* pTarget);
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    private:
        Poco::Net::HTTPRequestHandler* m_pTarget;
    };
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_HTTPTESTREQUESTHANDLERFACTORY_H_INCLUDED */
