/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COREVERSION_H_INCLUDED
#define EASYHTTPCPP_COMMON_COREVERSION_H_INCLUDED

#include <string>

namespace easyhttpcpp {
namespace common {

class ProjectVersion {
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
