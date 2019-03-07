/*
 * Copyright 2018 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForSharedPtrStream.h"
#include "RequestBodyUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForSharedPtrStream";

RequestBodyForSharedPtrStream::RequestBodyForSharedPtrStream(MediaType::Ptr pMediaType,
        Poco::SharedPtr<std::istream> pContent) : RequestBody(pMediaType), m_pContent(pContent)
{
    if (!m_pContent) {
        EASYHTTPCPP_LOG_D(Tag, "pContent cannot be NULL.");
        throw HttpIllegalArgumentException("pContent cannot be NULL.");
    }
}

RequestBodyForSharedPtrStream::~RequestBodyForSharedPtrStream()
{
}

void RequestBodyForSharedPtrStream::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(*m_pContent, outStream);
}

bool RequestBodyForSharedPtrStream::hasContentLength() const
{
    return false;
}

ssize_t RequestBodyForSharedPtrStream::getContentLength() const
{
    return -1;
}

bool RequestBodyForSharedPtrStream::reset()
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
