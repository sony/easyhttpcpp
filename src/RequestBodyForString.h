/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYFORSTRING_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYFORSTRING_H_INCLUDED

#include <string>

#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/RequestBody.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API RequestBodyForString : public RequestBody {
public:
    RequestBodyForString(MediaType::Ptr pMediaType, const std::string& content);
    virtual ~RequestBodyForString();
    virtual void writeTo(std::ostream& outStream);
    virtual bool hasContentLength() const;
    virtual ssize_t getContentLength() const;
    virtual bool reset();
private:
    const std::string* m_pContent;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYFORSTRING_H_INCLUDED */
