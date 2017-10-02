/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODY_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODY_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/MediaType.h"
#include "easyhttpcpp/ResponseBodyStream.h"

namespace easyhttpcpp {

/**
 * @brief A ResponseBody preserve Http response body.
 */
class ResponseBody : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<ResponseBody> Ptr;

    /**
     * 
     */
    virtual ~ResponseBody();

    /**
     * @brief Create ResponseBody
     * @param pMediaType MediaType
     * @param hasContentLength If exist content length, set true.
     * @param contentLength Content length. If not exist content length, set -1.
     * @param pContent response body stream
     * @return ResponseBody
     */
    static ResponseBody::Ptr create(MediaType::Ptr pMediaType, bool hasContentLength, ssize_t contentLength,
            ResponseBodyStream::Ptr pContent);

    /**
     * @brief Close ResponseBody.
     * 
     * after close, can not read response body.
     */
    virtual void close();

    /**
     * @brief Get response body stream.
     * @return ResponseBosyStream
     */
    virtual ResponseBodyStream::Ptr getByteStream() const;

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
     * @brief Get MediaType
     * @return MediaType
     */
    virtual MediaType::Ptr getMediaType() const;

    /**
     * @brief Get response body by string.
     * 
     * after toString, can not read response body.
     * @return  string
     * @exception HttpIllegalStateException
     * @exception HttpExecutionException
     */
    virtual std::string toString();

private:
    ResponseBody(MediaType::Ptr pMediaType, bool hasContentLength, ssize_t contentLength,
            ResponseBodyStream::Ptr pContent);

    MediaType::Ptr m_pMediaType;
    bool m_hasContentLength;
    ssize_t m_contentLength;
    ResponseBodyStream::Ptr m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODY_H_INCLUDED */
