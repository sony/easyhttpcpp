/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "Poco/Exception.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"

#include "ResponseBodyStreamInternal.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "ResponseBodyStreamInternal";
static const Poco::Timestamp::TimeDiff ResponseBodySkipTimeout = 100 * 1000;    // micro sec. 100ms
static const size_t ResponseBodySkipBytes = 8192;

ResponseBodyStreamInternal::ResponseBodyStreamInternal(std::istream& content) : m_closed(false), m_content(content)
{
}

ResponseBodyStreamInternal::~ResponseBodyStreamInternal()
{
    close();
}

ssize_t ResponseBodyStreamInternal::read(char* pBuffer, size_t readBytes)
{
    {
        Poco::Mutex::ScopedLock lock(m_instanceMutex);
        if (m_closed) {
            std::string message = "stream already closed.";
            EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
            throw HttpIllegalStateException(message);
        }
    }
    if (pBuffer == NULL) {
        std::string message = "pBuffer is NULL.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }
    if (readBytes > SSIZE_MAX) {
        std::string message = StringUtil::format("readBytes is over SSIZE_MAX. [readBytes=%zu]", readBytes);
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }

    try {
        if (isEof()) {
            return -1;
        }
        m_content.read(pBuffer, readBytes);
        if (!m_content && !m_content.eof()) {
            EASYHTTPCPP_LOG_D(Tag, "read: istream::read failed.[fail:%d, bad:%d]", m_content.fail(), m_content.bad());
            throw HttpExecutionException("istream::read failed");
        }

        ssize_t retBytes = m_content.gcount();

        return retBytes;

    } catch (const Poco::Exception& e) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "read: Poco::Exception %s [%s]", message.c_str(), e.message().c_str());
        throw HttpExecutionException(message, e);
    } catch (const std::exception& e) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "read: std::exception %s [%s]", message.c_str(), e.what());
        throw HttpExecutionException(message, e);
    } catch (...) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "read: unknown exception %s", message.c_str());
        throw HttpExecutionException(message);
    }
}

bool ResponseBodyStreamInternal::isEof()
{
    {
        Poco::Mutex::ScopedLock lock(m_instanceMutex);
        if (m_closed) {
            std::string message = "stream already closed.";
            EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
            throw HttpIllegalStateException(message);
        }
    }
    try {
        return m_content.eof();
    } catch (const Poco::Exception& e) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "isEof: Poco::Exception %s [%s]", message.c_str(), e.message().c_str());
        throw HttpExecutionException(message, e);
    } catch (const std::exception& e) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "isEof: std::exception %s [%s]", message.c_str(), e.what());
        throw HttpExecutionException(message, e);
    } catch (...) {
        std::string message = "can not read from stream.";
        EASYHTTPCPP_LOG_D(Tag, "isEof: unknown exception %s", message.c_str());
        throw HttpExecutionException(message);
    }
}

void ResponseBodyStreamInternal::close()
{
    Poco::Mutex::ScopedLock lock(m_instanceMutex);

    if (m_closed) {
        return;
    }

    m_closed = true;
}

bool ResponseBodyStreamInternal::skipAll(PocoHttpClientSessionPtr pPocoHttpClientSession)
{
    {
        Poco::Mutex::ScopedLock lock(m_instanceMutex);

        if (m_closed) {
            return false;
        }
    }

    if (isEof()) {
        return true;
    }

    Poco::Net::StreamSocket& socket = pPocoHttpClientSession->socket();
    Poco::Timespan originalTimeout;
    try {
        originalTimeout = socket.getReceiveTimeout();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_W(Tag, "skipAll: Could not get socket receive timeout.");
        EASYHTTPCPP_LOG_D(Tag, "skipAll: Could not get socket receive timeout. Details: %s", e.message().c_str());
        return false;
    }

    ssize_t skipBytes = 0;
    Poco::Timestamp startTime;
    Poco::Buffer<char> buffer(ResponseBodySkipBytes);
    bool ret = true;
    try {
        while (!isEof()) {
            Poco::Timestamp::TimeDiff remainingTime = ResponseBodySkipTimeout - startTime.elapsed();
            if (remainingTime <= 0) {
                EASYHTTPCPP_LOG_D(Tag, "skipAll: Could not skip all response body in time.");
                ret = false;
                break;
            }
            Poco::Timespan timeout(remainingTime);
            try {
                socket.setReceiveTimeout(timeout);
            } catch (const Poco::Exception& e) {
                EASYHTTPCPP_LOG_W(Tag, "skipAll: Could not set socket receive timeout for skip.");
                EASYHTTPCPP_LOG_D(Tag, "skipAll: Could not set socket receive timeout for skip. Details: %s",
                        e.message().c_str());
                ret = false;
                break;
            }
            ssize_t retBytes = read(buffer.begin(), ResponseBodySkipBytes);
            if (retBytes > 0) {
                skipBytes += retBytes;
            }
        }
        EASYHTTPCPP_LOG_D(Tag, "skipAll: ResponseBody skip bytes:%zd", skipBytes);
    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_W(Tag, "SkipAll: Could not skip stream since read of response body failed.");
        EASYHTTPCPP_LOG_D(Tag, "SkipAll: Could not skip stream since read of response body failed. Details: %s",
                e.getMessage().c_str());
        ret = false;
    }

    // reset receive timeout
    try {
        socket.setReceiveTimeout(originalTimeout);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_W(Tag, "skipAll: Could not reset socket receive timeout to original.");
        EASYHTTPCPP_LOG_D(Tag, "skipAll: Could not reset socket receive timeout to original. Details: %s",
                e.message().c_str());
        ret = false;
    }

    return ret;
}

} /* namespace easyhttpcpp */
