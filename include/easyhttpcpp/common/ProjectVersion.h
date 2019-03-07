/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COREVERSION_H_INCLUDED
#define EASYHTTPCPP_COMMON_COREVERSION_H_INCLUDED

#include <string>

#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API ProjectVersion {
public:
    static std::string getMajor();
    static std::string getMinor();
    static std::string getPatch();
    static std::string getExtension();
    static std::string asString();

private:
    ProjectVersion();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COREVERSION_H_INCLUDED */
