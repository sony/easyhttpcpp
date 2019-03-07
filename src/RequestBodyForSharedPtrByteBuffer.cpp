/*
 * Copyright 2018 Sony Corporation
 */

#include <limits.h>

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForSharedPtrByteBuffer.h"
#include "RequestBodyUtil.h"

#if defined(_WIN64)
#define SSIZE_MAX _I64_MAX
#elif defined(_WIN32)
#define SSIZE_MAX LONG_MAX
#endif

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForSharedPtrByteBuffer";

RequestBodyForSharedPtrByteBuffer::RequestBodyForSharedPtrByteBuffer(MediaType::Ptr pMediaType,
        Poco::SharedPtr<easyhttpcpp::common::ByteArrayBuffer> pContent) : RequestBody(pMediaType), m_pContent(pContent)
{
    if (!m_pContent) {
        EASYHTTPCPP_LOG_D(Tag, "pContent cannot be NULL.");
        throw HttpIllegalArgumentException("pContent cannot be NULL.");
    }
    if (m_pContent->getWrittenDataSize() > SSIZE_MAX) {
        EASYHTTPCPP_LOG_D(Tag, "Buffer size: [%zu] is too long.", m_pContent->getWrittenDataSize());
        throw HttpIllegalArgumentException("Buffer size is too long.");
    }
}

RequestBodyForSharedPtrByteBuffer::~RequestBodyForSharedPtrByteBuffer()
{
}

void RequestBodyForSharedPtrByteBuffer::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(*m_pContent, outStream);
}

bool RequestBodyForSharedPtrByteBuffer::hasContentLength() const
{
    return true;
}

ssize_t RequestBodyForSharedPtrByteBuffer::getContentLength() const
{
    return m_pContent->getWrittenDataSize();
}

bool RequestBodyForSharedPtrByteBuffer::reset()
{
    return true;
}

} /* namespace easyhttpcpp */
