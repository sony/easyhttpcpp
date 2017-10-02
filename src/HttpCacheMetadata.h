/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEMETADATA_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEMETADATA_H_INCLUDED

#include <ctime>
#include <string>

#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/Request.h"

namespace easyhttpcpp {

class HttpCacheMetadata : public easyhttpcpp::common::CacheMetadata {
public:
    HttpCacheMetadata();
    ~HttpCacheMetadata();

    void setUrl(const std::string& url);
    const std::string& getUrl() const;
    void setHttpMethod(Request::HttpMethod httpMethod);
    Request::HttpMethod getHttpMethod() const;
    void setStatusCode(int statusCode);
    int getStatusCode() const;
    void setStatusMessage(const std::string& statusMessage);
    const std::string& getStatusMessage() const;
    void setResponseHeaders(Headers::Ptr pResponseHeaders);
    Headers::Ptr getResponseHeaders() const;
    void setResponseBodySize(size_t responseBodySize);
    size_t getResponseBodySize() const;
    void setSentRequestAtEpoch(std::time_t sentRequestAtEpoch);
    std::time_t getSentRequestAtEpoch() const;
    void setReceivedResponseAtEpoch(std::time_t receivedResponseAtEpoch);
    std::time_t getReceivedResponseAtEpoch() const;
    void setCreatedAtEpoch(std::time_t createdAtEpoch);
    std::time_t getCreatedAtEpoch() const;

private:
    std::string m_url;
    Request::HttpMethod m_httpMethod;
    int m_statusCode;
    std::string m_statusMessage;
    Headers::Ptr m_pResponseHeaders;
    size_t m_responseBodySize;
    std::time_t m_sentRequestAtEpoch;
    std::time_t m_receivedResponseAtEpoch;
    std::time_t m_createdAtEpoch;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEMETADATA_H_INCLUDED */
