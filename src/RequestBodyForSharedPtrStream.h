/*
 * Copyright 2018 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTREAM_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTREAM_H_INCLUDED

#include <istream>

#include "Poco/SharedPtr.h"

#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class RequestBodyForSharedPtrStream : public RequestBody {
public:
    RequestBodyForSharedPtrStream(MediaType::Ptr pMediaType, Poco::SharedPtr<std::istream> pContent);
    virtual ~RequestBodyForSharedPtrStream();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    Poco::SharedPtr<std::istream> m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTREAM_H_INCLUDED */
