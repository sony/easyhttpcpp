/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINE_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINE_H_INCLUDED

#include <string>
#include <vector>

#include "Poco/AutoPtr.h"
#include "Poco/SharedPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/Typedef.h"
#include "easyhttpcpp/messagedigest/MessageDigestExports.h"

namespace easyhttpcpp {
namespace messagedigest {

class DigestEngineImpl;

class EASYHTTPCPP_MESSAGEDIGEST_API DigestEngine : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<DigestEngine> Ptr;

    DigestEngine(const std::string& algorithm);
    virtual ~DigestEngine();

    static std::string digestToHex(const std::vector<unsigned char>& digest);
    virtual void update(const easyhttpcpp::common::Byte* data, size_t length);
    virtual std::vector<unsigned char> digest();
    virtual void reset();

private:
    Poco::SharedPtr<DigestEngineImpl> m_pImpl;
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTENGINE_H_INCLUDED */
