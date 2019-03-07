/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COREEXPORTS_H_INCLUDED
#define EASYHTTPCPP_COMMON_COREEXPORTS_H_INCLUDED

namespace easyhttpcpp {
namespace common {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
#ifdef EASYHTTPCPP_API_EXPORTS
    #define EASYHTTPCPP_COMMON_API_EXPORTS
    #define EASYHTTPCPP_DB_API_EXPORTS
    #define EASYHTTPCPP_EXECUTORSERVICE_API_EXPORTS
    #define EASYHTTPCPP_HTTP_API_EXPORTS
    #define EASYHTTPCPP_MESSAGEDIGEST_API_EXPORTS
    #endif
#endif

} /* namespace easyhttpcpp */
} /* namespace common */

#endif /* EASYHTTPCPP_COMMON_COREEXPORTS_H_INCLUDED */

