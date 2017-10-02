/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Buffer.h"
#include "Poco/Exception.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/HttpException.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {

static const std::string Tag = "ResponseBody";
static const size_t DefaultBufferSize = 4096;

ResponseBody::ResponseBody(MediaType::Ptr pMediaType, bool hasContentLength, ssize_t contentLength,
        ResponseBodyStream::Ptr pContent) : m_pMediaType(pMediaType), m_hasContentLength(hasContentLength),
        m_contentLength(contentLength), m_pContent(pContent)
{
}

ResponseBody::~ResponseBody()
{
    close();
}

ResponseBody::Ptr ResponseBody::create(MediaType::Ptr pMediaType, bool hasContentLength, ssize_t contentLength,
        ResponseBodyStream::Ptr pContent)
{
    return new ResponseBody(pMediaType, hasContentLength, contentLength, pContent);
}

void ResponseBody::close()
{
    m_pContent->close();
}

ResponseBodyStream::Ptr ResponseBody::getByteStream() const
{
    return m_pContent;
}

bool ResponseBody::hasContentLength() const
{
    return m_hasContentLength;
}

ssize_t ResponseBody::getContentLength() const
{
    return m_contentLength;
}

MediaType::Ptr ResponseBody::getMediaType() const
{
    return m_pMediaType;
}

std::string ResponseBody::toString()
{
    if (m_contentLength == 0) {
        return "";
    }
    size_t bufferSize = DefaultBufferSize;
    if (m_contentLength > 0) {
        bufferSize = m_contentLength;
    }
    try {
        Poco::Buffer<char> inBuffer(bufferSize);
        ByteArrayBuffer outBuffer;
        while(!m_pContent->isEof()) {
            ssize_t retBytes = m_pContent->read(inBuffer.begin(), bufferSize);
            if (retBytes > 0) {
                outBuffer.write(reinterpret_cast<Byte*>(inBuffer.begin()), retBytes);
            }
        }
        m_pContent->close();
        return outBuffer.toString();

    } catch (const Poco::Exception& e) {
        std::string message = "IO exception occurred.";
        EASYHTTPCPP_LOG_D(Tag, "%s [%s]", message.c_str(), e.message().c_str());
        throw HttpExecutionException(message, e);
    }
}

} /* namespace easyhttpcpp */
