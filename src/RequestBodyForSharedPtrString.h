/*
 * Copyright 2018 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTRING_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTRING_H_INCLUDED

#include <string>

#include "Poco/SharedPtr.h"

#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API RequestBodyForSharedPtrString : public RequestBody {
public:
    RequestBodyForSharedPtrString(MediaType::Ptr pMediaType, Poco::SharedPtr<std::string> pContent);
    virtual ~RequestBodyForSharedPtrString();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    Poco::SharedPtr<std::string> m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORSHAREDPTRSTRING_H_INCLUDED */
