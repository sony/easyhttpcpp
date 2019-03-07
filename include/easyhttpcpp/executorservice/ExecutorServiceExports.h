/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXPORTS_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXPORTS_H_INCLUDED

#ifdef EASYHTTPCPP_API_EXPORTS
#include "easyhttpcpp/common/CoreExports.h"
#endif

namespace easyhttpcpp {
namespace executorservice {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
    #ifdef EASYHTTPCPP_EXECUTORSERVICE_API_EXPORTS
    #define EASYHTTPCPP_EXECUTORSERVICE_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_EXECUTORSERVICE_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_EXECUTORSERVICE_API
#define EASYHTTPCPP_EXECUTORSERVICE_API
#endif

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_EXECUTORSERVICEEXPORTS_H_INCLUDED */

