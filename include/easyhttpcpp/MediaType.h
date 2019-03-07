/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MEDIATYPE_H_INCLUDED
#define EASYHTTPCPP_MEDIATYPE_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

/**
 * @brief A MediaType preserve Content-Type.
 */
class EASYHTTPCPP_HTTP_API MediaType : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<MediaType> Ptr;

    /**
     * 
     * @param contentType Content-Type
     */
    MediaType(const std::string& contentType);

    /**
     * 
     */
    virtual ~MediaType();

    /**
     * @brief Get Content-Type by string
     * @return string
     */
    virtual std::string toString();
private:
    MediaType();
    std::string m_contentType;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MEDIATYPE_H_INCLUDED */
