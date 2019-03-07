/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>

#include "Poco/DateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeParser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPRequest.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/messagedigest/DigestUtil.h"

#include "HttpUtil.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::messagedigest::DigestUtil;

namespace easyhttpcpp {

static const std::string Tag = "HttpUtil";

const std::string& HttpUtil::httpMethodToString(Request::HttpMethod httpMethod)
{
    switch (httpMethod) {
        case Request::HttpMethodDelete:
            return Poco::Net::HTTPRequest::HTTP_DELETE;
        case Request::HttpMethodGet:
            return Poco::Net::HTTPRequest::HTTP_GET;
        case Request::HttpMethodHead:
            return Poco::Net::HTTPRequest::HTTP_HEAD;
        case Request::HttpMethodPatch:
            return Poco::Net::HTTPRequest::HTTP_PATCH;
        case Request::HttpMethodPost:
            return Poco::Net::HTTPRequest::HTTP_POST;
        case Request::HttpMethodPut:
            return Poco::Net::HTTPRequest::HTTP_PUT;
        default:
            throw HttpIllegalArgumentException(StringUtil::format("Unknown http method: [%d]", httpMethod));
    }
}

bool HttpUtil::tryParseDate(const std::string& value, Poco::Timestamp& timeStamp)
{
    // correspond 3 format of RFC2616 3.3.1
    //   Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123   -> Poco::DateTimeFormat::HTTP_FORMAT
    //   Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036 -> Poco::DateTimeFormat::HTTP_FORMAT
    //   Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format      -> Poco::DateTimeFormat::ASCTIME_FORMAT

    Poco::DateTime dateTime;
    int timeZone;
    if (!Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::HTTP_FORMAT, value, dateTime, timeZone)) {
        if (!Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::ASCTIME_FORMAT, value, dateTime, timeZone)) {
            EASYHTTPCPP_LOG_D(Tag, "can not parse header date/time [%s]", value.c_str());
            return false;
        }
    }

    timeStamp = dateTime.timestamp();
    return true;
}

std::string HttpUtil::makeCacheKey(Request::Ptr pRequest)
{
    return makeCacheKey(pRequest->getMethod(), pRequest->getUrl());
}

std::string HttpUtil::makeCacheKey(Request::HttpMethod httpMethod, const std::string& url)
{
    // method + url -> digest
    std::string key = DigestUtil::createHashedFileName(httpMethodToString(httpMethod) + url);
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "makeCacheKey failed.(empty string)");
    }
    return key;
}

std::string HttpUtil::makeCachedResponseBodyFilename(const Poco::Path& cacheRootDir, const std::string& key)
{
    Poco::Path filename(cacheRootDir, key + HttpConstants::Caches::DataFileExtention);
    return filename.toString();
}

Headers::Ptr HttpUtil::exchangeJsonStrToHeaders(const std::string& headerJsonStr)
{
    try {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(headerJsonStr);

        Poco::JSON::Object::Ptr headerJson = result.extract<Poco::JSON::Object::Ptr>();

        Headers::Ptr pHeaders = new Headers();
        for (Poco::JSON::Object::ConstIterator it = headerJson->begin(); it != headerJson->end(); it++) {
            pHeaders->add(it->first, it->second);
        }
        return pHeaders;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "exchangeJsonStrToHeaders failed. error:[%s]", e.message().c_str());
        return NULL;
    }
}

std::string HttpUtil::exchangeHeadersToJsonStr(Headers::Ptr pHeaders)
{
    try {
        Poco::JSON::Object headerJson;
        for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
            headerJson.set(it->first, it->second);
        }
        std::stringstream strStream;
        headerJson.stringify(strStream);
        return strStream.str();

    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "exchangeHeadersToJsonStr failed. error:[%s]", e.message().c_str());
        return "";
    }
}

} /* namespace easyhttpcpp */
