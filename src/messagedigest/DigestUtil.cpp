/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/messagedigest/DigestConstants.h"
#include "easyhttpcpp/messagedigest/DigestUtil.h"
#include "easyhttpcpp/messagedigest/MessageDigestException.h"

#include "DigestUtilImpl.h"

using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace messagedigest {

static const std::string Tag = "DigestUtil";

DigestUtil::DigestUtil()
{
}

DigestUtil::~DigestUtil()
{
}

std::string DigestUtil::sha256Hex(const ByteArrayBuffer& data)
{
    try {
        DigestEngine digestEngine(DigestConstants::MessageDigestAlgorithmSha256);
        digestEngine.update(data.getBuffer(), data.getWrittenDataSize());

        return DigestEngine::digestToHex(digestEngine.digest());
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "sha256Hex(): Failed to create hash. Error: %s", e.getMessage().c_str());

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
        DigestEngine digestEngine(DigestConstants::MessageDigestAlgorithmSha1);
        digestEngine.update(data.getBuffer(), data.getWrittenDataSize());

        return DigestEngine::digestToHex(digestEngine.digest());
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "sha1Hex(): Failed to create hash. Error: %s", e.getMessage().c_str());

        return "";
    }
}

std::string DigestUtil::sha1Hex(const std::string& data)
{
    ByteArrayBuffer dataBuffer(data);
    return sha1Hex(dataBuffer);
}

std::string DigestUtil::createHashedFileName(const std::string& key)
{
    return DigestUtilImpl::createHashedFileName(key);
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

