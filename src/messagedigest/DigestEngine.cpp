/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/messagedigest/DigestEngine.h"
#include "easyhttpcpp/messagedigest/MessageDigestException.h"

#include "DigestEngineImpl.h"

using easyhttpcpp::common::Byte;

namespace easyhttpcpp {
namespace messagedigest {

static const std::string Tag = "DigestEngine";

DigestEngine::DigestEngine(const std::string& algorithm)
{
    try {
        m_pImpl = new DigestEngineImpl(algorithm);
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while creating DigestEngine. Error:%s", e.getMessage().c_str());
        throw;
    }
}

DigestEngine::~DigestEngine()
{
}

std::string DigestEngine::digestToHex(const std::vector<unsigned char>& digest)
{
    try {
        return DigestEngineImpl::digestToHex(digest);
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while converting the digest. Error:%s", e.getMessage().c_str());
        throw;
    }
}

void DigestEngine::update(const Byte* data, size_t length)
{
    try {
        m_pImpl->update(data, length);
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while updating the digest. Error:%s", e.getMessage().c_str());
        throw;
    }
}

std::vector<unsigned char> DigestEngine::digest()
{
    try {
        return m_pImpl->digest();
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while calculating the digest. Error:%s", e.getMessage().c_str());
        throw;
    }
}

void DigestEngine::reset()
{
    try {
        m_pImpl->reset();
    } catch (const MessageDigestException& e) {
        EASYHTTPCPP_LOG_D(Tag, "Error occurred while resetting the engine. Error: %s:%s", e.getMessage().c_str());
        throw;
    }
}

} /* namespace messagedigest */
} /* namespace easyhttpcpp */
