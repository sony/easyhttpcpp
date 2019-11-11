/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_FILEUTILIMPL_H_INCLUDED
#define EASYHTTPCPP_COMMON_FILEUTILIMPL_H_INCLUDED

namespace easyhttpcpp {
namespace common {

class FileUtilImpl {
public:
    static std::string convertToAbsolutePathString(const std::string& path, bool extendedPrefix);
private:
    FileUtilImpl();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_FILEUTILIMPL_H_INCLUDED */

