/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_EXCEPTIONCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_COMMON_EXCEPTIONCONSTANTS_H_INCLUDED

#include <string>

namespace easyhttpcpp {
namespace common {

class ExceptionConstants {
public:
    static const std::string ErrorCodePrefix;

    /**
     * Exception group codes
     */
    class GroupCode {
    public:
        static const unsigned int Core;
    };

    /**
     * Exception subgroup codes
     */
    class SubGroupCode {
    public:
        static const unsigned int Common;
        static const unsigned int Db;
        static const unsigned int MessageDigest;
	static const unsigned int ExecutorService;
        static const unsigned int Http;
    };

};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_EXCEPTIONCONSTANTS_H_INCLUDED */
