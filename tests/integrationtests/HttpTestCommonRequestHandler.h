/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCOMMONREQUESTHANDLER_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCOMMONREQUESTHANDLER_H_INCLUDED

#include "Poco/Event.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "HttpTestBaseRequestHandler.h"

namespace easyhttpcpp {
namespace test {

class HttpTestCommonRequestHandler {
public:

    class OkRequestHandler : public HttpTestBaseRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class OneHourMaxAgeRequestHandler : public Poco::Net::HTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class BadRequestHandler : public HttpTestBaseRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class NotModifiedResponseRequestHandler1st : public Poco::Net::HTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class NotModifiedResponseRequestHandler2nd : public Poco::Net::HTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class WaitRequestHandler : public HttpTestBaseRequestHandler {
    public:
        typedef Poco::AutoPtr<WaitRequestHandler> Ptr;

        ~WaitRequestHandler();
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
        void set();
        bool waitForStart(long timeoutMilliSec);
    private:
        Poco::Event m_startEvent;
        Poco::Event m_waitEvent;
    };

    class ConnectionCloseRequestHandler : public HttpTestBaseRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class NoConnectionRequestHandler : public HttpTestBaseRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };

    class SpecifyingContentLengthRequestHandler : public Poco::Net::HTTPRequestHandler {
    public:
        SpecifyingContentLengthRequestHandler(ssize_t contentLength);
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    private:
        SpecifyingContentLengthRequestHandler();
        ssize_t m_contentLength;
    };

    class WaitInTheMiddleOfWriteResponseBodyRequestHandler : public Poco::Net::HTTPRequestHandler {
    public:
        WaitInTheMiddleOfWriteResponseBodyRequestHandler(ssize_t contentLength, ssize_t writeBytesBeforeSleep,
                long sleepMilliSec);
        virtual ~WaitInTheMiddleOfWriteResponseBodyRequestHandler();
        virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
        bool wait();

    private:
        WaitInTheMiddleOfWriteResponseBodyRequestHandler();

        ssize_t m_contentLength;
        ssize_t m_writeBytesBeforeSleep;
        long m_sleepMilliSec;
        bool m_needToWaitHandler;
        Poco::Event m_finishEvent;
    };

private:
    HttpTestCommonRequestHandler();
    virtual ~HttpTestCommonRequestHandler();

};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCOMMONREQUESTHANDLER_H_INCLUDED */
