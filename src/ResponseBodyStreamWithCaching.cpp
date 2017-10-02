/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/File.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpCacheInternal.h"
#include "HttpCacheMetadata.h"
#include "HttpUtil.h"
#include "ResponseBodyStreamWithCaching.h"

using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheManager;
using easyhttpcpp::common::CacheMetadata;

namespace easyhttpcpp {

static const std::string Tag = "ResponseBodyStreamWithCaching";

ResponseBodyStreamWithCaching::ResponseBodyStreamWithCaching(std::istream& content,
        ConnectionInternal::Ptr pConnectionInternal, ConnectionPoolInternal::Ptr pConnectionPoolInternal,
        Response::Ptr pResponse, HttpCache::Ptr pHttpCache) :
        ResponseBodyStreamInternal(content), m_pConnectionInternal(pConnectionInternal),
        m_pConnectionPoolInternal(pConnectionPoolInternal), m_pResponse(pResponse), m_pHttpCache(pHttpCache),
        m_pTempFileStream(NULL), m_writtenDataSize(0)
{
}

ResponseBodyStreamWithCaching::~ResponseBodyStreamWithCaching()
{
    closeOutStream();
}

ssize_t ResponseBodyStreamWithCaching::read(char* pBuffer, size_t readBytes)
{
    ssize_t retBytes = ResponseBodyStreamInternal::read(pBuffer, readBytes);
    if (retBytes < 0) {
        return retBytes;
    }

    Poco::Mutex::ScopedLock lock(m_instanceMutex);
    if (m_closed) {
        EASYHTTPCPP_LOG_D(Tag, "read: already closed.");
        throw HttpIllegalStateException("stream already closed.");
    }

    // save temp file for cache
    if (!createTempFile()) {
        return retBytes;
    }

    try {
        m_pTempFileStream->write(static_cast<const char*> (pBuffer), static_cast<std::streamsize> (retBytes));
        m_writtenDataSize += retBytes;
    } catch (const std::exception& e) {
        std::string message = "can not receive response because IO error occurred.(write)";
        EASYHTTPCPP_LOG_D(Tag, "%s %s", message.c_str(), e.what());
    }

    return retBytes;
}

void ResponseBodyStreamWithCaching::close()
{
    Poco::Mutex::ScopedLock lock(m_instanceMutex);

    if (m_closed) {
        EASYHTTPCPP_LOG_D(Tag, "close: already closed");
        return;
    }

    // skip remaining data, if exist.
    if (skipAll(m_pConnectionInternal->getPocoHttpClientSession())) {
        EASYHTTPCPP_LOG_D(Tag, "Connection is reusable.");
        // release connection.
        m_pConnectionPoolInternal->releaseConnection(m_pConnectionInternal);
    } else {
        EASYHTTPCPP_LOG_D(Tag, "Could not reuse connection since skipping response body failed.");
        // remove connection.
        m_pConnectionPoolInternal->removeConnection(m_pConnectionInternal);
    }
    m_pConnectionInternal = NULL;
    m_pConnectionPoolInternal = NULL;

    bool responseBodyValid = isValidResponseBody();

    ResponseBodyStreamInternal::close();

    // put cache
    closeOutStream();

    if (responseBodyValid) {
        putCache();
    } else {
        EASYHTTPCPP_LOG_D(Tag, "remove TempFile, because response body is not valid");
        removeTempFile();
    }

    EASYHTTPCPP_LOG_D(Tag, "close: finished");
}

Connection::Ptr ResponseBodyStreamWithCaching::getConnection()
{
    return m_pConnectionInternal.unsafeCast<Connection>();
}

bool ResponseBodyStreamWithCaching::createTempFile()
{
    if (m_pTempFileStream) {
        return true;
    }

    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pHttpCache.get());
    try {
        const std::string& tempDirectory = pCacheInternal->getTempDirectory();
        m_tempFilePath = Poco::TemporaryFile::tempName(tempDirectory);
        EASYHTTPCPP_LOG_D(Tag, "tempFile=%s", m_tempFilePath.c_str());
    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_D(Tag, "can not get temp directory. HttpException=%s", e.getMessage().c_str());
        m_pTempFileStream = NULL;
        m_tempFilePath.clear();
        return false;
    }

    try {
        m_pTempFileStream = new std::ofstream(m_tempFilePath.c_str(),
                std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        m_writtenDataSize = 0;

    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_D(Tag, "can not create temp file. HttpException=%s", e.getMessage().c_str());
        m_pTempFileStream = NULL;
        m_tempFilePath.clear();
        return false;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "can not create temp file. Poco::Exception=%s", e.message().c_str());
        m_pTempFileStream = NULL;
        m_tempFilePath.clear();
        return false;
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "can not create temp file. std::exception=%s", e.what());
        m_pTempFileStream = NULL;
        m_tempFilePath.clear();
        return false;
    }
    return true;
}

void ResponseBodyStreamWithCaching::closeOutStream()
{
    if (m_pTempFileStream) {
        m_pTempFileStream->close();
        delete m_pTempFileStream;
        m_pTempFileStream = NULL;
    }
}

bool ResponseBodyStreamWithCaching::isValidResponseBody()
{
    if (!m_pResponse->hasContentLength()) {
        if (isEof()) {
            return true;
        } else {
            return false;
        }
    }
    ssize_t contentLength = m_pResponse->getContentLength();
    if (contentLength == -1) {
        if (isEof()) {
            return true;
        } else {
            return false;
        }
    }
    if (contentLength != m_writtenDataSize) {
        return false;
    }
    return true;
}

void ResponseBodyStreamWithCaching::removeTempFile()
{
    if (m_tempFilePath.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "removeTempFile: temporary file is not created.");
        return;
    }
    try {
        Poco::File file(m_tempFilePath);
        if (file.exists()) {
            EASYHTTPCPP_LOG_D(Tag, "removeTempFile: filename=%s", m_tempFilePath.c_str());
            file.remove();
        }
    } catch (const Poco::Exception& e) {
        std::string message = "can not receive response because IO error occurred.(remove)";
        EASYHTTPCPP_LOG_D(Tag, "%s %s", message.c_str(), e.message().c_str());
        throw HttpExecutionException(message, e);
    }
}

void ResponseBodyStreamWithCaching::putCache()
{
    CacheMetadata::Ptr pCacheMetadata = new HttpCacheMetadata();
    HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*> (pCacheMetadata.get());
    Request::Ptr pRequest = m_pResponse->getRequest();
    pHttpCacheMetadata->setKey(HttpUtil::makeCacheKey(pRequest));
    if (pHttpCacheMetadata->getKey().empty()) {
        EASYHTTPCPP_LOG_D(Tag, "putCache: can not make key.");
        removeTempFile();
        return;
    }
    pHttpCacheMetadata->setUrl(pRequest->getUrl());
    pHttpCacheMetadata->setHttpMethod(pRequest->getMethod());
    pHttpCacheMetadata->setStatusCode(m_pResponse->getCode());
    pHttpCacheMetadata->setStatusMessage(m_pResponse->getMessage());
    pHttpCacheMetadata->setResponseHeaders(m_pResponse->getHeaders());
    pHttpCacheMetadata->setResponseBodySize(m_writtenDataSize);
    pHttpCacheMetadata->setSentRequestAtEpoch(m_pResponse->getSentRequestSec());
    pHttpCacheMetadata->setReceivedResponseAtEpoch(m_pResponse->getReceivedResponseSec());
    Poco::Timestamp now;
    pHttpCacheMetadata->setCreatedAtEpoch(now.epochTime());

    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pHttpCache.get());
    CacheManager::Ptr pCacheManager = pCacheInternal->getCacheManager();
    if (!pCacheManager->put(pHttpCacheMetadata->getKey(), pCacheMetadata, m_tempFilePath)) {
        EASYHTTPCPP_LOG_D(Tag, "putCache: can not put cache[%s].", pHttpCacheMetadata->getKey().c_str());
        removeTempFile();
    }
}


} /* namespace easyhttpcpp */
