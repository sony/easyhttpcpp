/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTUTILEXPORTS_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTUTILEXPORTS_H_INCLUDED

namespace easyhttpcpp {
namespace testutil {

#if defined(_WIN32) && defined(EASYHTTPCPP_DLL_TESTS)
    #ifdef EASYHTTPCPP_TESTUTIL_API_EXPORTS
    #define EASYHTTPCPP_TESTUTIL_API __declspec(dllexport)
    #else
    #define EASYHTTPCPP_TESTUTIL_API __declspec(dllimport)
    #endif
#endif

#ifndef EASYHTTPCPP_TESTUTIL_API
#define EASYHTTPCPP_TESTUTIL_API
#endif

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTUTILEXPORTS_H_INCLUDED */

