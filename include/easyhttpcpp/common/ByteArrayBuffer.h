/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_BYTEARRAYBUFFER_H_INCLUDED
#define EASYHTTPCPP_COMMON_BYTEARRAYBUFFER_H_INCLUDED

#include <string>

#include "easyhttpcpp/common/CommonExports.h"
#include "easyhttpcpp/common/Typedef.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API ByteArrayBuffer {
public:
    ByteArrayBuffer();
    ByteArrayBuffer(size_t capacity);
    ByteArrayBuffer(const std::string& src);
    virtual ~ByteArrayBuffer();

    Byte* getBuffer() const;
    size_t getBufferSize() const;
    size_t getWrittenDataSize() const;
    void setWrittenDataSize(size_t newWrittenDataSize);
    size_t write(const Byte* pBuffer, size_t size);
    void clear();
    std::string toString() const;

    bool copyTo(ByteArrayBuffer& dst) const;
    bool copyFrom(const std::string& src);
    bool copyFrom(const ByteArrayBuffer& src);

protected:
    Byte* expand(size_t newBufferSize);

    Byte* m_pBuffer;
    size_t m_bufferSize;
    size_t m_writtenDataSize;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_BYTEARRAYBUFFER_H_INCLUDED */
