/*
 * Copyright 2018 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForSharedPtrString.h"
#include "RequestBodyUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForSharedPtrString";

RequestBodyForSharedPtrString::RequestBodyForSharedPtrString(MediaType::Ptr pMediaType,
        Poco::SharedPtr<std::string> pContent) : RequestBody(pMediaType), m_pContent(pContent)
{
    if (!m_pContent) {
        EASYHTTPCPP_LOG_D(Tag, "pContent cannot be NULL.");
        throw HttpIllegalArgumentException("pContent cannot be NULL.");
    }
}

RequestBodyForSharedPtrString::~RequestBodyForSharedPtrString()
{
}

void RequestBodyForSharedPtrString::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(*m_pContent, outStream);
}

bool RequestBodyForSharedPtrString::hasContentLength() const
{
    return true;
}

ssize_t RequestBodyForSharedPtrString::getContentLength() const
{
    return m_pContent->size();
}

bool RequestBodyForSharedPtrString::reset()
{
    return true;
}

} /* namespace easyhttpcpp */
