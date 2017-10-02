/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_RESPONSEBODYSTREAM_H_INCLUDED
#define EASYHTTPCPP_RESPONSEBODYSTREAM_H_INCLUDED

#include <istream>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

namespace easyhttpcpp {

/**
 * @brief A ResponseBodyStream preserve Http response body stream.
 */
class ResponseBodyStream : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<ResponseBodyStream> Ptr;

    virtual ~ResponseBodyStream()
    {
    }

    /**
     * @brief Read response body date from stream
     * @param pBuffer read buffer
     * @param readBytes request to read bytes
     * @return actually read bytes. If is eof, return -1.
     * @exception HttpIllegalStateException
     * @exception HttpExecutionException
     */
    virtual ssize_t read(char* pBuffer, size_t readBytes) = 0;

    /**
     * @brief check eof
     * @return If eof, return true.
     * @exception HttpIllegalStateException
     */
    virtual bool isEof() = 0;

    /**
     * @brief Close stream.
     * 
     * after close, can not read response body.
     */
    virtual void close() = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_RESPONSEBODYSTREAM_H_INCLUDED */
