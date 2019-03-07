/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTCONSTANTS_H_INCLUDED

#include <string>

#include "easyhttpcpp/messagedigest/MessageDigestExports.h"

namespace easyhttpcpp {
namespace messagedigest {

class EASYHTTPCPP_MESSAGEDIGEST_API DigestConstants {
public:
    static const std::string MessageDigestAlgorithmSha256;
    static const std::string MessageDigestAlgorithmSha1;
    static const std::string MessageDigestAlgorithmMd5;
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTCONSTANTS_H_INCLUDED */
