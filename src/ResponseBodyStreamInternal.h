/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODYSTREAMINTERNAL_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODYSTREAMINTERNAL_H_INCLUDED

#include <istream>

#include "Poco/Mutex.h"

#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/ResponseBodyStream.h"

#include "HttpTypedefs.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API ResponseBodyStreamInternal : public ResponseBodyStream {
public:
    ResponseBodyStreamInternal(std::istream& content);
    virtual ~ResponseBodyStreamInternal();

    virtual ssize_t read(char* pBuffer, size_t readBytes);
    virtual bool isEof();
    virtual void close();

protected:
    virtual bool skipAll(PocoHttpClientSessionPtr pPocoHttpClientSession);

    Poco::Mutex m_instanceMutex;
    bool m_closed;
    std::istream& m_content;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODYSTREAMINTERNAL_H_INCLUDED */
