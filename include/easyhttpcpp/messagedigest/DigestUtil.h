/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED

#include <string>

#include "Poco/Mutex.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/messagedigest/DigestEngine.h"
#include "easyhttpcpp/messagedigest/MessageDigestExports.h"

namespace easyhttpcpp {
namespace messagedigest {

class EASYHTTPCPP_MESSAGEDIGEST_API DigestUtil {
public:
    static std::string sha256Hex(const easyhttpcpp::common::ByteArrayBuffer& data);
    static std::string sha256Hex(const std::string& data);

    static std::string sha1Hex(const easyhttpcpp::common::ByteArrayBuffer& data);
    static std::string sha1Hex(const std::string& data);

    static std::string createHashedFileName(const std::string& key);

private:
    DigestUtil();
    virtual ~DigestUtil();
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED */
