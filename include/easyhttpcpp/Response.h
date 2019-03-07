/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSE_H_INCLUDED
#define EASYHTTPCPP_RESPONSE_H_INCLUDED

#include <ctime>
#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/CacheControl.h"
#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/ResponseBody.h"

namespace easyhttpcpp {

/**
 * @brief A Response preserve Http response parameters.
 */
class EASYHTTPCPP_HTTP_API Response : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Response> Ptr;

    class Builder;

    Response(Response::Ptr pResponse);

    /**
     * 
     */
    virtual ~Response();

    /**
     * @brief Get response body
     * @return ResponseBody
     */
    virtual ResponseBody::Ptr getBody() const;

    /**
     * @brief Get CacheControl
     * @return CacheControl
     */
    virtual CacheControl::Ptr getCacheControl();

    /**
     * @brief Get response status code.
     * @return status code
     */
    virtual int getCode() const;

    /**
     * @brief Get response status message
     * @return 
     */
    virtual const std::string& getMessage() const;

    /**
     * @brief check ContentLength
     * @return if exist content length, return true.
     */
    virtual bool hasContentLength() const;

    /**
     * @brief Get ContentLength
     * @return ContentLength. if not exist content length, return -1.
     */
    virtual ssize_t getContentLength() const;

    /**
     * @brief Get response header value
     * @param name name to find
     * @param defaultValue if name is found, default value is returned.
     * @return value
     */
    virtual const std::string& getHeaderValue(const std::string& name, const std::string& defaultValue) const;

    /**
     * @brief Check response header
     * @param name name to check
     * @return if exist true.
     */
    virtual bool hasHeader(const std::string& name) const;

    /**
     * @Get response header
     * @return Header
     */
    virtual Headers::Ptr getHeaders() const;

    /**
     * @brief Get Request corresponding Response
     * @return Request
     */
    virtual Request::Ptr getRequest() const;

    /**
     * @brief Get Cache Response
     * @return Cache response
     */
    virtual Response::Ptr getCacheResponse() const;

    /**
     * @brief Get Network Response
     * @return Network response
     */
    virtual Response::Ptr getNetworkResponse() const;

    /**
     * @brief Get Prior Response
     * @return Prior response
     */
    virtual Response::Ptr getPriorResponse() const;

    virtual std::time_t getSentRequestSec() const;

    virtual std::time_t getReceivedResponseSec() const;

    /**
     * @brief check http response
     * @return If http response is successful, return true.
     */
    virtual bool isSuccessful() const;

    /**
     * @brief Get http response by string
     * @return string
     */
    virtual std::string toString();

private:
    Response(Response::Builder& builder);
    void initFromBuilder(Builder& builder);

    int m_statusCode;
    std::string m_statusMessage;
    bool m_hasContentLength;
    ssize_t m_contentLength;
    Headers::Ptr m_pHeaders;
    CacheControl::Ptr m_pCacheControl;
    ResponseBody::Ptr m_pBody;
    Request::Ptr m_pRequest;
    Response::Ptr m_pCacheResponse;
    Response::Ptr m_pNetworkResponse;
    Response::Ptr m_pPriorResponse;
    std::time_t m_sentRequestSec;
    std::time_t m_receivedResponseSec;

public:

    /**
     * @brief A Request::Builder is Builder for Request.
     */
    class EASYHTTPCPP_HTTP_API Builder {
    public:
        /**
         * 
         */
        Builder();

        Builder(Response::Ptr pResponse);

        /**
         * 
         * @return 
         */
        virtual ~Builder();

        /**
         * @brief Build Request by specified parameters.
         * @return Response
         */
        Response::Ptr build();

        /**
         * @brief Set responseBody
         * @param pBody ResponseBody
         * @return ResponseBody
         */
        Builder& setBody(ResponseBody::Ptr pBody);

        /**
         * @brief Get responseBody
         * @return ResponseBody
         */
        ResponseBody::Ptr getBody() const;

        /**
         * @brief Set status code
         * @param code status code
         * @return Builder
         */
        Builder& setCode(int code);

        /**
         * @brief Get status codel
         * @return status code
         */
        int getCode() const;

        /**
         * @brief Set status message
         * @param message status message
         * @return Builder
         */
        Builder& setMessage(const std::string& message);

        /**
         * @brief Get status message
         * @return status message
         */
        const std::string& getMessage() const;

        /**
         * @brief Set to have contentLength
         * @param hasContentLength if exist content length, return true.
         * @return Builder
         */
        Builder& setHasContentLength(bool hasContentLength);

        /**
         * @brief check ContentLength
         * @return if exist content length, return true.
         */
        bool hasContentLength() const;

        /**
         * @brief Set ContentLength
         * @param contentLength. if not exist content length, return -1.
         */
        Builder& setContentLength(ssize_t contentLength);

        /**
         * @brief Get ContentLength
         * @return ContentLength. if not exist content length, return -1.
         */
        ssize_t getContentLength() const;

        /**
         * @brief Set CacheControl
         * @param pCacheControl CacheControl
         * @return Builder
         */
        Builder& setCacheControl(CacheControl::Ptr pCacheControl);

        /**
         * @brief Get CacheControl
         * @return CacheControl
         */
        CacheControl::Ptr getCacheControl() const;

        /**
         * @brief Set response header element.
         * @param name name to set
         * @param value value
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setHeader(const std::string& name, const std::string& value);

        /**
         * @brief Add response header element.
         * @param name name to set
         * @param value value
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& addHeader(const std::string& name, const std::string& value);

        /**
         * @brief Set response headers
         * @param pHeaders
         * @return 
         */
        Builder& setHeaders(Headers::Ptr pHeaders);

        /**
         * @brief Get response headers
         * @return Header
         */
        Headers::Ptr getHeaders() const;

        /**
         * @brief Set Request
         * @param pRequest Request
         * @return Builder
         */
        Builder& setRequest(Request::Ptr pRequest);

        /**
         * @brief Get Request
         * @return Request
         */
        Request::Ptr getRequest();

        /**
         * @brief Set cache response
         * @param pResponse Cache response
         * @return Cache response
         */
        Builder& setCacheResponse(Response::Ptr pResponse);

        /**
         * @brief Get cache Response
         * @return Cache response
         */
        Response::Ptr getCacheResponse() const;

        /**
         * @brief Get network response
         * @param pResponse Network response
         * @return Network response
         */
        Builder& setNetworkResponse(Response::Ptr pResponse);

        /**
         * @brief Get Network Response
         * @return Network response
         */
        Response::Ptr getNetworkResponse() const;

        /**
         * @brief Get prior response
         * @param pResponse Prior response
         * @return Prior response
         */
        Builder& setPriorResponse(Response::Ptr pResponse);

        /**
         * @brief Get Prior response
         * @return Prior response
         */
        Response::Ptr getPriorResponse() const;

        Builder& setSentRequestSec(std::time_t sentRequestSec);
        std::time_t getSentRequestSec() const;

        Builder& setReceivedResponseSec(std::time_t receivedResponseSec);
        std::time_t getReceivedResponseSec() const;

    private:
        int m_statusCode;
        std::string m_statusMessage;
        bool m_hasContentLength;
        ssize_t m_contentLength;
        Headers::Ptr m_pHeaders;
        CacheControl::Ptr m_pCacheControl;
        ResponseBody::Ptr m_pBody;
        Request::Ptr m_pRequest;
        Response::Ptr m_pCacheResponse;
        Response::Ptr m_pNetworkResponse;
        Response::Ptr m_pPriorResponse;
        std::time_t m_sentRequestSec;
        std::time_t m_receivedResponseSec;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSE_H_INCLUDED */
