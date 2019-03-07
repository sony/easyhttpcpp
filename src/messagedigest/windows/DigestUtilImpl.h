/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTILIMPL_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTILIMPL_H_INCLUDED

#include <string>

namespace easyhttpcpp {
namespace messagedigest {

class DigestUtilImpl {
public:
    static std::string createHashedFileName(const std::string& key);
private:
    DigestUtilImpl();
};

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_DIGESTUTILIMPL_H_INCLUDED */
