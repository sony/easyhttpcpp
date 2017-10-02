/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODYSTREAMWITHCACHING_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODYSTREAMWITHCACHING_H_INCLUDED

#include <fstream>

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/HttpCache.h"
#include "easyhttpcpp/Response.h"

#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "HttpEngine.h"
#include "ResponseBodyStreamInternal.h"

namespace easyhttpcpp {

class ResponseBodyStreamWithCaching : public ResponseBodyStreamInternal {
public:
    ResponseBodyStreamWithCaching(std::istream& content, ConnectionInternal::Ptr pConnectionInternal,
            ConnectionPoolInternal::Ptr pConnectionPoolInternal, Response::Ptr pResponse, HttpCache::Ptr pHttpCache);
    virtual ~ResponseBodyStreamWithCaching();
    virtual ssize_t read(char* pBuffer, size_t readBytes);
    virtual void close();

    Connection::Ptr getConnection();    // for test

private:
    bool createTempFile();
    void closeOutStream();
    bool isValidResponseBody();
    void removeTempFile();
    void putCache();
    
    ConnectionInternal::Ptr m_pConnectionInternal;
    ConnectionPoolInternal::Ptr m_pConnectionPoolInternal;
    Response::Ptr m_pResponse;
    HttpCache::Ptr m_pHttpCache;
    std::string m_tempFilePath;
    std::ofstream* m_pTempFileStream;
    ssize_t m_writtenDataSize;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODYSTREAMWITHCACHING_H_INCLUDED */
