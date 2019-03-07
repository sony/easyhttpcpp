/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXPORTS_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXPORTS_H_INCLUDED

#ifdef EASYHTTPCPP_API_EXPORTS
#include "easyhttpcpp/common/CoreExports.h"
#endif

namespace easyhttpcpp {
namespace messagedigest {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
    #ifdef EASYHTTPCPP_MESSAGEDIGEST_API_EXPORTS
    #define EASYHTTPCPP_MESSAGEDIGEST_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_MESSAGEDIGEST_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_MESSAGEDIGEST_API
#define EASYHTTPCPP_MESSAGEDIGEST_API
#endif

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXPORTS_H_INCLUDED */

