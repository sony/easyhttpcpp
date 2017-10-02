/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCONSTANTS_H_INCLUDED

#include <stddef.h>

namespace easyhttpcpp {
namespace test {

class HttpTestConstants {
public:
    static const char* const DefaultTestUrlWithQuery;
    static const char* const DefaultTestUrl;
    static const char* const DefaultHttpsTestUrl;
    static const char* const Http;
    static const char* const Https;
    static const char* const DefaultHost;
    static const unsigned short DefaultPort;
    static const unsigned short DefaultHttpsPort;
    static const char* const DefaultPath;
    static const char* const DefaultQuery;
    static const size_t DefaultCacheMaxSize;

    static const char* const DefaultResponseContentType;
    static const char* const DefaultResponseBody;
    static const char* const BadRequestResponseBody;
    static const char* const HeaderCacheControl;
    static const char* const MaxAgeOneHour; // 1 hour
    static const char* const HeaderLastModified;
    static const char* const HeaderValueLastModified;
    static const char* const DefaultRequestContentType;
    static const char* const DefaultRequestBody;
    static const char* const Chunked01ResponseBody;
    static const char* const Chunked02ResponseBody;

    static const char* const ExternalUrlHttpNoCacheControl;
    static const char* const ExternalUrlHttpNoCacheControlNoStore;
    static const char* const ExternalUrlHttps;
    static const char* const ExternalUrlHttps2;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPTESTCONSTANTS_H_INCLUDED */
