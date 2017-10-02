/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED

#include <string>

#include "Poco/Mutex.h"
#include "Poco/Crypto/DigestEngine.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"

namespace easyhttpcpp {
namespace messagedigest {

class DigestUtil {
public:
    static const std::string MessageDigestAlgorithmSha256;
    static const std::string MessageDigestAlgorithmSha1;

    static std::string sha256Hex(const easyhttpcpp::common::ByteArrayBuffer& data);
    static std::string sha256Hex(const std::string& data);

    static std::string sha1Hex(const easyhttpcpp::common::ByteArrayBuffer& data);
    static std::string sha1Hex(const std::string& data);

private:
    DigestUtil();
    virtual ~DigestUtil();
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTIL_H_INCLUDED */
