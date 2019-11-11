/*
 * Copyright 2017 Sony Corporation
 */

#include "TestConstants.h"

namespace easyhttpcpp {
namespace testutil {

const char* const TestConstants::AppIdDevEnclaveClient = "1008";
const char* const TestConstants::AnalyticsDispatchEndpointQaV1 = "https://bdcore-asq-lb.bda.csxdev.com/actionlog/";
const char* const TestConstants::AnalyticsDispatchEndpointQaV2 =
        "https://sr-logcollector-qas.alph.cloud/v1/direct/999999999999DUMMY/log";
const char* const TestConstants::LoaderDistributionBaseUrl = "http://d3d9bizloqaofq.cloudfront.net/";
const char* const TestConstants::LoaderDistributionCertificateUrl1008 =
        "https://d2w29nwp6bf7n5.cloudfront.net/certificates/1008";
const char* const TestConstants::BaseUrlSni = "http://cdn.meta.csxdev.com/";
const char* const TestConstants::CertificateUrlEnclaveConfiguration
        = "https://d2w29nwp6bf7n5.cloudfront.net/certificates";
const char* const TestConstants::CertificateUrlSni = "https://cert-cdn.meta.csxdev.com/certificates/1008";
const char* const TestConstants::LongDirName64byte = "012345678901234567890123456789012345678901234567890LongDirName64";

} /* namespace testutil */
} /* namespace easyhttpcpp */
