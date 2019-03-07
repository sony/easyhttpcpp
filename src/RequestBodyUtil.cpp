/*
 * Copyright 2018 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyUtil.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyUtil";
static const size_t ReadBufferSize = 4096;

void RequestBodyUtil::write(const ByteArrayBuffer& inBuffer, std::ostream& outStream)
{
    try {
        outStream.write(reinterpret_cast<const char*>(inBuffer.getBuffer()), inBuffer.getWrittenDataSize());
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write ByteArrayBuffer to ostream. Poco::Exception: [%s]", e.message().c_str());
        throw HttpExecutionException("Cannot write ByteArrayBuffer to ostream. Check getCause() for details.", e);
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write ByteArrayBuffer to ostream. std::exception: [%s]", e.what());
        throw HttpExecutionException("Cannot write ByteArrayBuffer to ostream. Check getCause() for details.", e);
    }
}

void RequestBodyUtil::write(std::istream& inStream, std::ostream& outStream)
{
    try {
        Poco::SharedPtr< char, Poco::ReferenceCounter, Poco::ReleaseArrayPolicy<char> >
                pBuffer(new char[ReadBufferSize]);
        while (!inStream.eof()) {
            inStream.read(pBuffer, ReadBufferSize);
            ssize_t readBytes = inStream.gcount();
            outStream.write(pBuffer, readBytes);
        }
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write istream to ostream. Poco::Exception: [%s]", e.message().c_str());
        throw HttpExecutionException("Cannot write istream to ostream. Check getCause() for details.", e);
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write istream to ostream. std::exception: [%s]", e.what());
        throw HttpExecutionException("Cannot write istream to ostream. Check getCause() for details.", e);
    }
}

void RequestBodyUtil::write(const std::string& inBuffer, std::ostream& outStream)
{
    try {
        outStream << inBuffer;
        outStream.flush();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write string to ostream. Poco::Exception: [%s]", e.message().c_str());
        throw HttpExecutionException("Cannot write string to ostream. Check getCause() for details.", e);
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Cannot write string to ostream. std::exception: [%s]", e.what());
        throw HttpExecutionException("Cannot write string to ostream. Check getCause() for details.", e);
    }
}

} /* namespace easyhttpcpp */
