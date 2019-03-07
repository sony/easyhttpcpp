/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_OPENSSLINITIALIZER_H_INCLUDED
#define EASYHTTPCPP_COMMON_OPENSSLINITIALIZER_H_INCLUDED

#ifndef _WIN32
#include "Poco/Crypto/OpenSSLInitializer.h"
#endif // !_WIN32

namespace easyhttpcpp {
namespace common {

class OpenSslInitializer {
public:
    OpenSslInitializer()
    {
    }

#ifndef _WIN32
private:
    Poco::Crypto::OpenSSLInitializer m_openSslInitializer;
#endif // !_WIN32
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_OPENSSLINITIALIZER_H_INCLUDED */
