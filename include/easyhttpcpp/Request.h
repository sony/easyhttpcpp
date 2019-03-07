/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUEST_H_INCLUDED
#define EASYHTTPCPP_REQUEST_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/HashMap.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/CacheControl.h"
#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

/**
 * @brief A Request preserve Http request parameters.
 */
class EASYHTTPCPP_HTTP_API Request : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Request> Ptr;

    class Builder;

    enum HttpMethod {
        HttpMethodDelete = 0, /**< Http Method Delete */
        HttpMethodGet, /**< Http Method GET */
        HttpMethodHead, /**< Http Method Head */
        HttpMethodPatch, /**< Http Method Patch */
        HttpMethodPost, /**< Http Method Post */
        HttpMethodPut /**< Http Method Put */
    };

    /**
     * @param builder Builder
     * @exception HttpIllegalArgumentException
     */
    Request(Request::Builder& builder);

    Request(Request::Ptr pRequest);

    /**
     * 
     */
    virtual ~Request();

    /**
     * @brief Get RequestBody
     * @return RequestBody
     */
    virtual RequestBody::Ptr getBody() const;

    /**
     * @brief Get CacheControl
     * @return CacheControl
     */
    virtual CacheControl::Ptr getCacheControl() const;

    /**
     * @brief Get request header value
     * @param name name to find
     * @param defaultValue if name is found, default value is returned.
     * @return value
     */
    virtual const std::string& getHeaderValue(const std::string& name, const std::string& defaultValue) const;

    /**
     * @brief Check request header
     * @param name name to check
     * @return if exist true.
     */
    virtual bool hasHeader(const std::string& name) const;

    /**
     * @brief Get request header
     * @return Header
     */
    virtual Headers::Ptr getHeaders() const;

    /**
     * @brief Get http method
     * @return http method
     */
    virtual HttpMethod getMethod() const;

    /**
     * @brief Get tag
     * @return tag
     */
    virtual const void* getTag() const;

    /**
     * @brief Get url
     * @return url
     */
    virtual const std::string& getUrl() const;

private:
    void initFromBuilder(Builder& builder);

    HttpMethod m_method;
    std::string m_url;
    const void* m_pTag;
    Headers::Ptr m_pHeaders;
    CacheControl::Ptr m_pCacheControl;
    RequestBody::Ptr m_pBody;

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

        /**
         * 
         * @param request
         */
        Builder(Request::Ptr pRequest);

        /**
         * 
         * @return 
         */
        virtual ~Builder();

        /**
         * @brief Build Request by specified parameters.
         * @return Request
         */
        Request::Ptr build();

        /**
         * @brief Request Delete Method
         * @return Builder
         */
        Builder& httpDelete();

        /**
         * @brief Request Delete Method
         * @param pBody request body
         * @return Builder
         */
        Builder& httpDelete(RequestBody::Ptr pBody);

        /**
         * @brief Request Get Method
         * 
         * default of http method is get method.
         * @return Builder
         */
        Builder& httpGet();

        /**
         * @brief Request Head Method
         * @return Builder
         */
        Builder& httpHead();

        /**
         * @brief Request Patch Method
         * @param pBody request body
         * @return Builder
         */
        Builder& httpPatch(RequestBody::Ptr pBody);

        /**
         * @brief Request Post Method
         * @param pBody request body
         * @return Builder
         */
        Builder& httpPost(RequestBody::Ptr pBody);

        /**
         * @brief Request Post Method
         * @return Builder
         */
        Builder& httpPost();

        /**
         * @brief Request Put Method
         * @param pBody request body
         * @return Builder
         */
        Builder& httpPut(RequestBody::Ptr pBody);

        /**
         * @brief Get requested method.
         * @return method
         */
        HttpMethod getMethod() const;

        /**
         * @brief Get RequestBody
         * @return RequestBody
         */
        RequestBody::Ptr getBody() const;

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
         * @brief Set request header element.
         * @param name name to set
         * @param value value
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setHeader(const std::string& name, const std::string& value);

        /**
         * @brief Set request headers
         * @param pHeaders
         * @return 
         */
        Builder& setHeaders(Headers::Ptr pHeaders);

        /**
         * @Get request header
         * @return Header
         */
        Headers::Ptr getHeaders() const;

        /**
         * @brief Set tag.
         * @param pTag tag
         * @return Builder
         */
        Builder& setTag(const void* pTag);

        /**
         * :brief Get tag.
         * @return tag
         */
        const void* getTag() const;

        /**
         * @brief Set http request url
         * @param url url
         * @return Builder
         * @exception HttpIllegalArgumentException
         */
        Builder& setUrl(const std::string& url);

        /**
         * @brief Get url
         * @return url
         */
        const std::string& getUrl() const;

    private:
        HttpMethod m_method;
        std::string m_url;
        const void* m_pTag;
        Headers::Ptr m_pHeaders;
        CacheControl::Ptr m_pCacheControl;
        RequestBody::Ptr m_pBody;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUEST_H_INCLUDED */
