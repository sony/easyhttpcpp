/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"

#include "easyhttpcpp/messagedigest/DigestUtil.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace messagedigest {

static const std::string Tag = "DigestUtil";

const std::string DigestUtil::MessageDigestAlgorithmSha256 = "SHA256";
const std::string DigestUtil::MessageDigestAlgorithmSha1 = "SHA1";

DigestUtil::DigestUtil()
{
}

DigestUtil::~DigestUtil()
{
}

std::string DigestUtil::sha256Hex(const ByteArrayBuffer& data)
{
    try {
        Poco::Crypto::DigestEngine digestEngine(MessageDigestAlgorithmSha256);
        digestEngine.update(data.getBuffer(), data.getWrittenDataSize());

        return Poco::Crypto::DigestEngine::digestToHex(digestEngine.digest());
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "sha256Hex(): Failed to create hash. Error: %s", e.message().c_str());

        return "";
    }
}

std::string DigestUtil::sha256Hex(const std::string& data)
{
    ByteArrayBuffer dataBuffer(data);
    return sha256Hex(dataBuffer);
}

std::string DigestUtil::sha1Hex(const ByteArrayBuffer& data)
{
    try {
        Poco::Crypto::DigestEngine digestEngine(MessageDigestAlgorithmSha1);
        digestEngine.update(data.getBuffer(), data.getWrittenDataSize());

        return Poco::Crypto::DigestEngine::digestToHex(digestEngine.digest());
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "sha256Hex(): Failed to create hash. Error: %s", e.message().c_str());

        return "";
    }
}

std::string DigestUtil::sha1Hex(const std::string& data)
{
    ByteArrayBuffer dataBuffer(data);
    return sha1Hex(dataBuffer);
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

