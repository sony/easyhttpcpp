/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForString.h"
#include "RequestBodyUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForString";

RequestBodyForString::RequestBodyForString(MediaType::Ptr pMediaType, const std::string& content) :
        RequestBody(pMediaType), m_pContent(&content)
{
}

RequestBodyForString::~RequestBodyForString()
{
}

void RequestBodyForString::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(*m_pContent, outStream);
}

bool RequestBodyForString::hasContentLength() const
{
    return true;
}

ssize_t RequestBodyForString::getContentLength() const
{
    return m_pContent->size();
}

bool RequestBodyForString::reset()
{
    return true;
}

} /* namespace easyhttpcpp */
