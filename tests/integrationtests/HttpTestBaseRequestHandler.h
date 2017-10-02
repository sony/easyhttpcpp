/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTBASEREQUESTHANDLER_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTBASEREQUESTHANDLER_H_INCLUDED

#include <string>

#include "Poco/Buffer.h"
#include "Poco/SharedPtr.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NameValueCollection.h"

namespace easyhttpcpp {
namespace test {

class HttpTestBaseRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
    HttpTestBaseRequestHandler();
    virtual ~HttpTestBaseRequestHandler();

    void saveRequestParamemter(Poco::Net::HTTPServerRequest& request);
    void saveRequestParameterAsString(Poco::Net::HTTPServerRequest& request);
    void saveRequestParameterAsBinary(Poco::Net::HTTPServerRequest& request);
    void clearParameter();

    const std::string& getMethod() const;
    const Poco::URI& getUri() const;
    const Poco::Net::NameValueCollection& getRequestHeaders() const;
    const std::string& getRequestContentType() const;
    const std::string& getRequestBody() const;
    ssize_t getRequestContentLength() const;
    size_t getRequestBodyBytes() const;
    Poco::SharedPtr< Poco::Buffer<char> > getRequestBodyBuffer() const;
    void setMaxRequestBodyBufferSize(size_t maxRequestBodyBufferSize);

protected:
    std::string m_method;
    Poco::URI m_uri;
    Poco::Net::NameValueCollection m_requestHeaders;
    bool m_hasRequestContentLength;
    ssize_t m_requestContentLength;
    std::string m_requestBody;
    Poco::SharedPtr< Poco::Buffer<char> > m_pRequestBodyBuffer;
    size_t m_requestBodyBytes;
    std::string m_requestContentType;
    size_t m_maxRequestBodyBufferSize;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTBASEREQUESTHANDLER_H_INCLUDED */
