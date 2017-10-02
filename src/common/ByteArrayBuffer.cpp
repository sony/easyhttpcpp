/*
 * Copyright 2017 Sony Corporation
 */

#include <string.h>
#include <exception>

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/CoreLogger.h"

namespace easyhttpcpp {
namespace common {

static const std::string Tag = "ByteArrayBuffer";

ByteArrayBuffer::ByteArrayBuffer() : m_pBuffer(NULL), m_bufferSize(0), m_writtenDataSize(0)
{
}

ByteArrayBuffer::ByteArrayBuffer(size_t capacity) : m_pBuffer(new Byte[capacity]), m_bufferSize(capacity),
m_writtenDataSize(0)
{
    memset(m_pBuffer, 0, m_bufferSize);
}

ByteArrayBuffer::ByteArrayBuffer(const std::string& src)
{
    m_bufferSize = src.size() + 1;
    m_writtenDataSize = src.size();

    m_pBuffer = new Byte[m_bufferSize];
    memcpy(m_pBuffer, src.c_str(), src.size());
    m_pBuffer[m_writtenDataSize] = '\0';
}

ByteArrayBuffer::~ByteArrayBuffer()
{
    delete [] m_pBuffer;
    m_pBuffer = NULL;
}

Byte* ByteArrayBuffer::getBuffer() const
{
    return m_pBuffer;
}

size_t ByteArrayBuffer::getBufferSize() const
{
    return m_bufferSize;
}

size_t ByteArrayBuffer::getWrittenDataSize() const
{
    return m_writtenDataSize;
}

void ByteArrayBuffer::setWrittenDataSize(size_t newWrittenDataSize)
{
    m_writtenDataSize = newWrittenDataSize;
}

size_t ByteArrayBuffer::write(const Byte* pBuffer, size_t size)
{
    size_t total = m_writtenDataSize + size;
    if (m_bufferSize < total) {
        if (expand(total) == NULL) {
            return 0;
        }
    }
    memcpy(m_pBuffer + m_writtenDataSize, pBuffer, size);
    m_writtenDataSize = total;
    return size;
}

Byte* ByteArrayBuffer::expand(size_t newbufferSize)
{
    if (newbufferSize == 0) {
        return NULL;
    }

    Byte* pNewBuffer = NULL;
    try {
        pNewBuffer = new Byte[newbufferSize];
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "expand failed with error : %s", e.what());
    }

    if (pNewBuffer == NULL) {
        return NULL;
    }

    memset(pNewBuffer, 0, newbufferSize);
    memcpy(pNewBuffer, m_pBuffer, (m_bufferSize > newbufferSize) ? newbufferSize : m_bufferSize);
    delete [] m_pBuffer;
    m_pBuffer = NULL;
    m_bufferSize = newbufferSize;
    m_pBuffer = pNewBuffer;
    return m_pBuffer;
}

void ByteArrayBuffer::clear()
{
    delete [] m_pBuffer;
    m_pBuffer = NULL;
    m_bufferSize = 0;
    m_writtenDataSize = 0;
}

std::string ByteArrayBuffer::toString() const
{

    if (m_bufferSize == 0 || m_writtenDataSize == 0 || m_pBuffer == NULL) {
        return "";
    }

    size_t size = m_writtenDataSize + 1;
    char* tmp = new char[size];
    if (tmp == NULL) {
        return "";
    }
    memcpy(tmp, m_pBuffer, m_writtenDataSize);
    tmp[m_writtenDataSize] = '\0';

    std::string str(tmp);
    delete [] tmp;
    tmp = NULL;

    return str;
}

bool ByteArrayBuffer::copyTo(ByteArrayBuffer& dst) const
{
    dst.clear();

    if (dst.expand(m_bufferSize) == NULL) {
        return false;
    }

    size_t size = dst.write(m_pBuffer, m_writtenDataSize);
    if (size != dst.getWrittenDataSize()) {
        return false;
    }

    return true;
}

bool ByteArrayBuffer::copyFrom(const std::string& src)
{
    clear();

    if (expand(src.size() + 1) == NULL) {
        return false;
    }

    size_t size = write(reinterpret_cast<Byte*> (const_cast<char*> ((src.c_str()))), src.size());
    if (size != getWrittenDataSize()) {
        return false;
    }

    return true;
}

bool ByteArrayBuffer::copyFrom(const ByteArrayBuffer& src)
{
    clear();

    if (expand(src.getBufferSize()) == NULL) {
        return false;
    }

    size_t size = write(src.getBuffer(), src.getWrittenDataSize());
    if (size != getWrittenDataSize()) {
        return false;
    }

    return true;
}

} /* namespace common */
} /* namespace easyhttpcpp */
