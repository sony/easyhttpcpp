/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COMMONEXPORTS_H_INCLUDED
#define EASYHTTPCPP_COMMON_COMMONEXPORTS_H_INCLUDED

#ifdef EASYHTTPCPP_API_EXPORTS
#include "easyhttpcpp/common/CoreExports.h"
#endif

namespace easyhttpcpp {
namespace common {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
    #ifdef EASYHTTPCPP_COMMON_API_EXPORTS
    #define EASYHTTPCPP_COMMON_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_COMMON_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_COMMON_API
#define EASYHTTPCPP_COMMON_API
#endif

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL_TESTS)
    #ifdef EASYHTTPCPP_COMMON_API_EXPORTS
    #define EASYHTTPCPP_CORE_COMMON_INTERNAL_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_CORE_COMMON_INTERNAL_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_CORE_COMMON_INTERNAL_API
#define EASYHTTPCPP_CORE_COMMON_INTERNAL_API
#endif

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COMMONEXPORTS_H_INCLUDED */

