/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "Poco/Exception.h"

#include "easyhttpcpp/common/BaseException.h"
#include "easyhttpcpp/common/CommonException.h"
#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {

const std::string BaseException::DefaultExceptionMessage = "Unknown exception occurred.";

BaseException::BaseException(const std::string& message)
{
}

BaseException::BaseException(const std::string& message, const std::exception& cause)
{
    if (dynamic_cast<const BaseException*> (&cause)) {
        // Clone easyhttpcpp exception.
        m_pCause = static_cast<const BaseException*> (&cause)->clone();
    } else if (dynamic_cast<const Poco::Exception*> (&cause)) {
        // Create PocoException.
        std::string messageOfPocoException = StringUtil::format("%s(%d) %s",
                static_cast<const Poco::Exception*> (&cause)->name(),
                static_cast<const Poco::Exception*> (&cause)->code(),
                static_cast<const Poco::Exception*> (&cause)->message().c_str());
        m_pCause = new PocoException(messageOfPocoException);
    } else {
        // Default create StdException.
        m_pCause = new StdException(cause.what());
    }
}

BaseException::BaseException(const BaseException& exception) : m_message(exception.m_message),
        m_pCause(exception.m_pCause)
{
}

BaseException::~BaseException() throw()
{
}

BaseException& BaseException::operator=(const BaseException& exception)
{
    if (&exception != this) {
        m_message = exception.m_message;
        m_pCause = exception.m_pCause;
    }
    return *this;
}

unsigned int BaseException::getCode() const
{
    return getExceptionGroupCode() * 10000 + getExceptionSubGroupCode() * 100 + getExceptionCode();
}

const std::string& BaseException::getMessage() const
{
    return m_message;
}

const char* BaseException::what() const throw()
{
    return m_message.c_str();
}

const BaseException::Ptr BaseException::getCause() const
{
    return m_pCause;
}

std::string BaseException::createExceptionMessage(const std::string& rawMessage)
{
    return StringUtil::format("%s%u: %s", ExceptionConstants::ErrorCodePrefix.c_str(), getCode(),
            rawMessage.empty() ? BaseException::DefaultExceptionMessage.c_str() :
            rawMessage.c_str());
}

} /* namespace common */
} /* namespace easyhttpcpp */
