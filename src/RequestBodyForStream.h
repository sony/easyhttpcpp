/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORSTREAM_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORSTREAM_H_INCLUDED

#include <istream>

#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class RequestBodyForStream : public RequestBody {
public:
    RequestBodyForStream(MediaType::Ptr pMediaType, std::istream& content);
    virtual ~RequestBodyForStream();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    std::istream* m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORSTREAM_H_INCLUDED */
