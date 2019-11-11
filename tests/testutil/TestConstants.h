/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTCONSTANTS_H_INCLUDED

#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

class EASYHTTPCPP_TESTUTIL_API TestConstants {
public:
    static const char* const AppIdDevEnclaveClient;
    static const char* const AnalyticsDispatchEndpointQaV1;
    static const char* const AnalyticsDispatchEndpointQaV2;
    static const char* const LoaderDistributionBaseUrl;
    static const char* const LoaderDistributionCertificateUrl1008;
    static const char* const BaseUrlSni;
    static const char* const CertificateUrlEnclaveConfiguration;
    static const char* const CertificateUrlSni;
    static const char* const LongDirName64byte;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTCONSTANTS_H_INCLUDED */
