/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODY_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODY_H_INCLUDED

#include <ostream>

#include "Poco/AutoPtr.h"
#include "Poco/Buffer.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/MediaType.h"

namespace easyhttpcpp {

/**
 * @brief A RequestBody preserve Http request body.
 */
class RequestBody : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<RequestBody> Ptr;

    /**
     * 
     */
    virtual ~RequestBody();

    /**
     * @brief Create RequestBody by stream
     * @param pMediaType MediaType
     * @param content request body stream
     * @return RequestBody
     */
    static Ptr create(MediaType::Ptr pMediaType, std::istream& content);

    /**
     * @brief Create RequestBody by string
     * @param pMediaType MediaType
     * @param content request body string
     * @return RequestBody
     */
    static Ptr create(MediaType::Ptr pMediaType, const std::string& content);

    /**
     * @brief Create RequestBody by BYteArrsyBuffer
     * @param pMediaType MediaType
     * @param content request body buffer
     * @return RequestBody
     * @exception HttpIllegalArgumentException
     */
    static Ptr create(MediaType::Ptr pMediaType, const easyhttpcpp::common::ByteArrayBuffer& content);

    /**
     * @brief Get MethiaType
     * @return MediaType
     */
    virtual MediaType::Ptr getMediaType() const;

    /**
     * @brief Write request body to stream
     * @param outStream stream to write
     * @exception HttpExecutionException
     */
    virtual void writeTo(std::ostream& outStream) = 0;

    /**
     * @brief check ContentLength
     * @return if exist content length, return true.
     */
    virtual bool hasContentLength() const = 0;

    /**
     * @brief Get ContentLength
     * @return ContentLength. if not exist content length, return -1.
     */
    virtual ssize_t getContentLength() const = 0;

    /**
     * @brief reset for re-read request body.
     * @return if succeeded to reset, return true.
     */
    virtual bool reset() = 0;

protected:
    virtual void setMediaType(MediaType::Ptr pMediaType);

private:
    MediaType::Ptr m_pMediaType;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODY_H_INCLUDED */
