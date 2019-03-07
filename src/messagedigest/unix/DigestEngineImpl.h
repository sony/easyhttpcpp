/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED

#include <string>
#include <vector>

#include "Poco/Crypto/DigestEngine.h"
#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/Typedef.h"

namespace easyhttpcpp {
namespace messagedigest {

class DigestEngineImpl {
public:
    DigestEngineImpl(const std::string& algorithm);
    virtual ~DigestEngineImpl();

    static std::string digestToHex(const std::vector<unsigned char>& digest);
    virtual void update(const easyhttpcpp::common::Byte* data, size_t length);
    std::vector<unsigned char> digest();
    virtual void reset();
private:

    Poco::SharedPtr<Poco::Crypto::DigestEngine> m_pDigestEngine;

};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED */
