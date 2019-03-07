/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/messagedigest/DigestConstants.h"
#include "easyhttpcpp/messagedigest/DigestUtil.h"
#include "easyhttpcpp/messagedigest/MessageDigestException.h"

#include "DigestEngineImpl.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace messagedigest {

static const std::string Tag = "DigestEngineImpl";

DigestEngineImpl::DigestEngineImpl(const std::string& algorithm)
{
    if (!CryptAcquireContext(&m_cspHandle, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        std::string message = StringUtil::format("Error occurred while acquiring crypto context. Details: 0x%08x",
            GetLastError());
        EASYHTTPCPP_LOG_D(Tag, message.c_str());
        throw MessageDigestExecutionException(message);
    }

    m_algorithmId = getAlgorithmIdFromString(algorithm);

    if (!CryptCreateHash(m_cspHandle, m_algorithmId, 0, 0, &m_hashHandle)) {
        std::string message = StringUtil::format("Error occurred while creating hash object. Details: 0x%08x",
            GetLastError());
        EASYHTTPCPP_LOG_D(Tag, message.c_str());
        throw MessageDigestExecutionException(message);
    }
}

DigestEngineImpl::~DigestEngineImpl()
{
    if (m_hashHandle) {
        CryptDestroyHash(m_hashHandle);
    }

    if (m_cspHandle) {
        CryptReleaseContext(m_cspHandle, 0);
    }
}

void DigestEngineImpl::update(const Byte* data, size_t length)
{
    if (!CryptHashData(m_hashHandle, static_cast<const BYTE*>(data), static_cast<DWORD>(length), 0)) {
        std::string message = StringUtil::format("Error occurred while updating the digest with given data. Details: 0x%08x",
            GetLastError());
        EASYHTTPCPP_LOG_D(Tag, message.c_str());
        throw MessageDigestExecutionException(message);
    }
}

void DigestEngineImpl::reset()
{
    if (m_hashHandle) {
        CryptDestroyHash(m_hashHandle);
    }
    if (!CryptCreateHash(m_cspHandle, m_algorithmId, 0, 0, &m_hashHandle)) {
        std::string message = StringUtil::format("Error occurred while resetting the engine. Details: 0x%08x",
            GetLastError());
        EASYHTTPCPP_LOG_D(Tag, message.c_str());
        throw MessageDigestExecutionException(message);
    }
}

std::vector<unsigned char> DigestEngineImpl::digest()
{
    // allocate the buffer which store the hash value.
    PBYTE pHashData = NULL;
    try {
        DWORD dataLength = 0;
        if (!CryptGetHashParam(m_hashHandle, HP_HASHVAL, NULL, &dataLength, 0)) {
            std::string message = StringUtil::format("Error occurred while calculating hash size. Details: 0x%08x",
                GetLastError());
            EASYHTTPCPP_LOG_D(Tag, message.c_str());
            throw MessageDigestExecutionException(message);
        }
        pHashData = (BYTE*)malloc(dataLength);
        if (!pHashData) {
            std::string message = StringUtil::format("Unable to allocate memory for hash calculation.");
            EASYHTTPCPP_LOG_D(Tag, message.c_str());
            throw MessageDigestExecutionException(message);
        }

        // get hash value
        if (!CryptGetHashParam(m_hashHandle, HP_HASHVAL, pHashData, &dataLength, 0)) {
            std::string message = StringUtil::format("Error occurred while calculating hash. Details: 0x%08x",
                GetLastError());
            EASYHTTPCPP_LOG_D(Tag, message.c_str());
            throw MessageDigestExecutionException(message);
        }

        std::vector<unsigned char> hashData;
        hashData.assign(&pHashData[0], &pHashData[dataLength]);

        // reset the engine
        reset();

        free(pHashData);
        return hashData;
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while calculating hash. Details: %s", e.getMessage().c_str());
        free(pHashData);
        throw;
    }
}

std::string DigestEngineImpl::digestToHex(const std::vector<unsigned char>& digest)
{
    static const char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(digest.size() * 2);
    for (std::vector<unsigned char>::const_iterator it = digest.begin(); it != digest.end(); ++it) {
        unsigned char c = *it;
        result += digits[(c >> 4) & 0xF];
        result += digits[c & 0xF];
    }
    return result;
}

unsigned int DigestEngineImpl::getAlgorithmIdFromString(const std::string algorithm)
{
    if (algorithm == DigestConstants::MessageDigestAlgorithmSha256) {
        return CALG_SHA_256;
    } else if (algorithm == DigestConstants::MessageDigestAlgorithmSha1) {
       return CALG_SHA1;
    } else if (algorithm == DigestConstants::MessageDigestAlgorithmMd5) {
        return CALG_MD5;
    } else {
        std::string message = StringUtil::format("Unsupported algorithm. algorithm: %s", algorithm.c_str());
        EASYHTTPCPP_LOG_D(Tag, message.c_str());
        throw MessageDigestExecutionException(message);
    }
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */
