/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForByteBuffer.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForByteBuffer";

RequestBodyForByteBuffer::RequestBodyForByteBuffer(MediaType::Ptr pMediaType,
        const easyhttpcpp::common::ByteArrayBuffer& content) : m_content(content)
{
    if (m_content.getWrittenDataSize() > SSIZE_MAX) {
        std::string message = "buffer size is too long.[" +
                Poco::NumberFormatter::format(m_content.getWrittenDataSize()) + "]";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    setMediaType(pMediaType);
}

RequestBodyForByteBuffer::~RequestBodyForByteBuffer()
{
}

void RequestBodyForByteBuffer::writeTo(std::ostream& outStream)
{
    try {
        outStream.write(reinterpret_cast<const char*>(m_content.getBuffer()), m_content.getWrittenDataSize());
    } catch (const Poco::Exception& e) {
        std::string message = "can not send request body.";
        EASYHTTPCPP_LOG_D(Tag, "Poco::Exception %s [%s]", message.c_str(), e.message().c_str());
        throw HttpExecutionException(message, e);
    } catch (const std::exception& e) {
        std::string message = "can not send request body.";
        EASYHTTPCPP_LOG_D(Tag, "std::exception %s [%s]", message.c_str(), e.what());
        throw HttpExecutionException(message, e);
    }
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
