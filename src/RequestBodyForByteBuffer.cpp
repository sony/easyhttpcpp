/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForByteBuffer.h"
#include "RequestBodyUtil.h"

#if defined(_WIN64)
#define SSIZE_MAX _I64_MAX
#elif defined(_WIN32)
#define SSIZE_MAX LONG_MAX
#endif

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForByteBuffer";

RequestBodyForByteBuffer::RequestBodyForByteBuffer(MediaType::Ptr pMediaType,
        const easyhttpcpp::common::ByteArrayBuffer& content) : RequestBody(pMediaType), m_content(content)
{
    if (m_content.getWrittenDataSize() > SSIZE_MAX) {
        EASYHTTPCPP_LOG_D(Tag, "Buffer size: [%zu] is too long.", m_content.getWrittenDataSize());
        throw HttpIllegalArgumentException("Buffer size is too long.");
    }
}

RequestBodyForByteBuffer::~RequestBodyForByteBuffer()
{
}

void RequestBodyForByteBuffer::writeTo(std::ostream& outStream)
{
    RequestBodyUtil::write(m_content, outStream);
}

bool RequestBodyForByteBuffer::hasContentLength() const
{
    return true;
}

ssize_t RequestBodyForByteBuffer::getContentLength() const
{
    return m_content.getWrittenDataSize();
}

bool RequestBodyForByteBuffer::reset()
{
    return true;
}

} /* namespace easyhttpcpp */
