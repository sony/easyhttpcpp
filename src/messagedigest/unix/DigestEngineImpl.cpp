/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Crypto/DigestEngine.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/messagedigest/MessageDigestException.h"

#include "DigestEngineImpl.h"

namespace easyhttpcpp {
namespace messagedigest {

static const std::string Tag = "DigestEngineImpl";

DigestEngineImpl::DigestEngineImpl(const std::string& algorithm)
{
    try {
        m_pDigestEngine = new Poco::Crypto::DigestEngine(algorithm);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while creating DigestEngine. error: %s", e.message().c_str());
        throw MessageDigestExecutionException("Error occurred while creating DigestEngine.", e);
    }
}

DigestEngineImpl::~DigestEngineImpl()
{
}

void DigestEngineImpl::update(const easyhttpcpp::common::Byte* data, std::size_t length)
{
    try {
        m_pDigestEngine->update(data, length);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while updating the digest. Error: %s", e.message().c_str());
        throw MessageDigestExecutionException("Error occurred while updating the digest.", e);
    }
}

void DigestEngineImpl::reset()
{
    try {
        m_pDigestEngine->reset();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while resetting the engine. Error: %s", e.message().c_str());
        throw MessageDigestExecutionException("Error occurred while resetting the engine.", e);
    }
}

std::vector<unsigned char> DigestEngineImpl::digest()
{
    try {
        return m_pDigestEngine->digest();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while calculating the digest. Error: %s", e.message().c_str());
        throw MessageDigestExecutionException("Error occurred while calculating the digest.", e);
    }
}

std::string DigestEngineImpl::digestToHex(const std::vector<unsigned char>& digest)
{
    try {
        return Poco::Crypto::DigestEngine::digestToHex(digest);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while converting the digest. Error: %s", e.message().c_str());
        throw MessageDigestExecutionException("Error occurred while converting the digest.", e);
    }
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */
