/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPEXPORTS_H_INCLUDED
#define EASYHTTPCPP_HTTPEXPORTS_H_INCLUDED

#ifdef EASYHTTPCPP_API_EXPORTS
#include "easyhttpcpp/common/CoreExports.h"
#endif

namespace easyhttpcpp {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
    #ifdef EASYHTTPCPP_HTTP_API_EXPORTS
    #define EASYHTTPCPP_HTTP_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_HTTP_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_HTTP_API
#define EASYHTTPCPP_HTTP_API
#endif

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL_TESTS)
    #ifdef EASYHTTPCPP_HTTP_API_EXPORTS
    #define EASYHTTPCPP_HTTP_INTERNAL_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_HTTP_INTERNAL_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_HTTP_INTERNAL_API
#define EASYHTTPCPP_HTTP_INTERNAL_API
#endif

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPEXPORTS_H_INCLUDED */

