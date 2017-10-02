/*
 * Copyright 2017 Sony Corporation
 */

#include "HttpTestConstants.h"

namespace easyhttpcpp {
namespace test {

const char* const HttpTestConstants::DefaultTestUrlWithQuery = "http://localhost:9982/path?a=10&b=20";
const char* const HttpTestConstants::DefaultTestUrl = "http://localhost:9982/path";
const char* const HttpTestConstants::DefaultHttpsTestUrl = "https://localhost:9444/path";
const char* const HttpTestConstants::Http = "http";
const char* const HttpTestConstants::Https = "https";
const char* const HttpTestConstants::DefaultHost = "localhost";
const unsigned short HttpTestConstants::DefaultPort = 9982;
const unsigned short HttpTestConstants::DefaultHttpsPort = 9444;
const char* const HttpTestConstants::DefaultPath = "/path";
const char* const HttpTestConstants::DefaultQuery = "a=10&b=20";
const size_t HttpTestConstants::DefaultCacheMaxSize = 1024;

const char* const HttpTestConstants::DefaultResponseContentType = "text/plain";
const char* const HttpTestConstants::DefaultResponseBody = "response data 1";
const char* const HttpTestConstants::BadRequestResponseBody = "Bad Request";
const char* const HttpTestConstants::HeaderCacheControl = "Cache-Control";
const char* const HttpTestConstants::MaxAgeOneHour = "max-age=3600"; // 1 hour
const char* const HttpTestConstants::HeaderLastModified = "Last-Modified";
const char* const HttpTestConstants::HeaderValueLastModified = "Mon, 25 Jul 2016 10:13:43 GMT";
const char* const HttpTestConstants::DefaultRequestContentType = "text/plain";
const char* const HttpTestConstants::DefaultRequestBody = "request body";
const char* const HttpTestConstants::Chunked01ResponseBody = "response body chunked01";
const char* const HttpTestConstants::Chunked02ResponseBody = "chunked02";

const char* const HttpTestConstants::ExternalUrlHttps = "https://github.com/sony";
const char* const HttpTestConstants::ExternalUrlHttps2 = "https://github.com/sony/easyhttpcpp";

} /* namespace test */
} /* namespace easyhttpcpp */
