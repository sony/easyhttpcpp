/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForStream.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForStream";
static const size_t ReadBufferSize = 4096;

RequestBodyForStream::RequestBodyForStream(MediaType::Ptr pMediaType, std::istream& content) : m_pContent(&content)
{
    setMediaType(pMediaType);
}

RequestBodyForStream::~RequestBodyForStream()
{
}

void RequestBodyForStream::writeTo(std::ostream& outStream)
{
    try {
        Poco::SharedPtr< char, Poco::ReferenceCounter, Poco::ReleaseArrayPolicy<char> >
                pBuffer(new char[ReadBufferSize]);
        while (!m_pContent->eof()) {
            m_pContent->read(pBuffer, ReadBufferSize);
            ssize_t readBytes = m_pContent->gcount();
            outStream.write(pBuffer, readBytes);
        }
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
