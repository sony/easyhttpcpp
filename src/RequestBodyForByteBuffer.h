/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORBYTEBUFFER_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORBYTEBUFFER_H_INCLUDED

#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class RequestBodyForByteBuffer : public RequestBody {
public:
    RequestBodyForByteBuffer(MediaType::Ptr pMediaType, const easyhttpcpp::common::ByteArrayBuffer& content);
    virtual ~RequestBodyForByteBuffer();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    const easyhttpcpp::common::ByteArrayBuffer& m_content;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORBYTEBUFFER_H_INCLUDED */
