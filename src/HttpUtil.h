/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPUTIL_H_INCLUDED
#define EASYHTTPCPP_HTTPUTIL_H_INCLUDED

#include <string>

#include "Poco/Path.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Request.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API HttpUtil {
public:
    static const std::string& httpMethodToString(Request::HttpMethod httpMethod);
    static bool tryParseDate(const std::string& value, Poco::Timestamp& timeStamp);
    static std::string makeCacheKey(Request::Ptr pRequest);
    static std::string makeCacheKey(Request::HttpMethod httpMethod, const std::string& url);
    static std::string makeCachedResponseBodyFilename(const Poco::Path& cacheRootDir, const std::string& key);
    static Headers::Ptr exchangeJsonStrToHeaders(const std::string& headerJsonStr);
    static std::string exchangeHeadersToJsonStr(Headers::Ptr pHeaders);
private:
    HttpUtil();
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPUTIL_H_INCLUDED */
