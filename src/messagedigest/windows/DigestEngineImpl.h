/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED

#include <string>
#include <vector>

#include "Windows.h"
#include "wincrypt.h"

namespace easyhttpcpp {
namespace messagedigest {

class DigestEngineImpl{
public:
    DigestEngineImpl(const std::string& algorithm);
    virtual ~DigestEngineImpl();

    virtual void update(const easyhttpcpp::common::Byte* data, size_t length);
    virtual void reset();
    virtual std::vector<unsigned char> digest();
    static std::string digestToHex(const std::vector<unsigned char>& digest);
private:
    unsigned int getAlgorithmIdFromString(const std::string algorithm);

    unsigned int m_algorithmId;

    // handle to a CSP
    HCRYPTPROV m_cspHandle;
	// handle to the new hash object
    HCRYPTHASH m_hashHandle;
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINEIMPL_H_INCLUDED */
