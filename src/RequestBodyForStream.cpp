/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForStream.h"
#include "RequestBodyUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForStream";

RequestBodyForStream::RequestBodyForStream(MediaType::Ptr pMediaType, std::istream& content) :
        RequestBody(pMediaType), m_pContent(&content)
{
}

RequestBodyForStream::~RequestBodyForStream()
{
}

void RequestBodyForStream::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(*m_pContent, outStream);
}

bool RequestBodyForStream::hasContentLength() const
{
    return false;
}

ssize_t RequestBodyForStream::getContentLength() const
{
    return -1;
}

bool RequestBodyForStream::reset()
{
    if (m_pContent->seekg(0, std::istream::beg).fail()) {
        EASYHTTPCPP_LOG_D(Tag, "reset: failed to reset.");
        return false;
    } else {
        EASYHTTPCPP_LOG_D(Tag, "reset: succeeded to reset.");
        return true;
    }
}

} /* namespace easyhttpcpp */
