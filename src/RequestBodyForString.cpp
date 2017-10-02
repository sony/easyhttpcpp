/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "RequestBodyForString.h"

namespace easyhttpcpp {

static const std::string Tag = "RequestBodyForString";

RequestBodyForString::RequestBodyForString(MediaType::Ptr pMediaType, const std::string& content)
{
    setMediaType(pMediaType);
    m_pContent = &content;
}

RequestBodyForString::~RequestBodyForString()
{
}

void RequestBodyForString::writeTo(std::ostream& outStream)
{
    try {
        outStream << *m_pContent;
        outStream.flush();
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
