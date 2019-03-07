/*
 * Copyright 2018 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORSHAREDPTRBYTEBUFFER_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORSHAREDPTRBYTEBUFFER_H_INCLUDED

#include "Poco/SharedPtr.h"

#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class RequestBodyForSharedPtrByteBuffer : public RequestBody {
public:
    RequestBodyForSharedPtrByteBuffer(MediaType::Ptr pMediaType,
        Poco::SharedPtr<easyhttpcpp::common::ByteArrayBuffer> pContent);
    virtual ~RequestBodyForSharedPtrByteBuffer();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    Poco::SharedPtr<easyhttpcpp::common::ByteArrayBuffer> m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORSHAREDPTRBYTEBUFFER_H_INCLUDED */
