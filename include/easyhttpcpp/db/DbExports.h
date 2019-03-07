/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_DBEXPORTS_H_INCLUDED
#define EASYHTTPCPP_DB_DBEXPORTS_H_INCLUDED

#ifdef EASYHTTPCPP_API_EXPORTS
#include "easyhttpcpp/common/CoreExports.h"
#endif

namespace easyhttpcpp {
namespace db {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL)
    #ifdef EASYHTTPCPP_DB_API_EXPORTS
    #define EASYHTTPCPP_DB_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_DB_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_DB_API
#define EASYHTTPCPP_DB_API
#endif

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_DBEXPORTS_H_INCLUDED */

