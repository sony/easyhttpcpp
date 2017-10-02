/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/DateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeParser.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "HttpCacheStrategy.h"
#include "HttpUtil.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "HttpCacheStrategyUnitTest";
static const char* const RequestUrl = "http://quiver.com/test1";
static const char* const RequetUrlQuery = "http://quiver.com/test1?a=10";
static const char* const HeaderDate = "Date";
static const char* const HeaderLastModified = "Last-Modified";
static const char* const HeaderExpires = "Expires";
static const char* const HeaderAge = "Age";
static const char* const HeaderWarning = "Warning";

class HttpCacheStrategyUnitTest : public testing::Test {
};

namespace {

bool isExistedWarning(Response::Ptr pResponse, const std::string& warningNo)
{
    Headers::Ptr pHeaders = pResponse->getHeaders();
    if (!pHeaders) {
        return false;
    }
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderWarning) == 0) {
            if (Poco::icompare(it->second, warningNo.length(), warningNo) == 0) {
                return true;
            }
        }
    }
    return false;
}
bool isNotExistedWarning(Response::Ptr pResponse, const std::string& warningNo)
{
    Headers::Ptr pHeaders = pResponse->getHeaders();
    if (!pHeaders) {
        return true;
    }
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        if (Poco::icompare(it->first, HeaderWarning) == 0) {
            if (Poco::icompare(it->second, warningNo.length(), warningNo) == 0) {
                return false;
            }
        }
    }
    return true;
}

} /* namespace */

TEST(HttpCacheStrategyUnitTest, constructor_getCachedResponseIsNull_WhenCacheResponseIsNull)
{
    // Given:
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(RequestUrl).build();

    // When: create HttpCacheStrategy
    Poco::Timestamp now;
    unsigned long long nowAtEpoch = static_cast<unsigned long long>(now.epochTime());
    HttpCacheStrategy httpCacheStrategy(pRequest, NULL, nowAtEpoch);

    // Then: networkRequest != NULL
    EXPECT_FALSE(httpCacheStrategy.getNetworkRequest().isNull());
    // cachedResponse == NULL
    EXPECT_TRUE(httpCacheStrategy.getCachedResponse().isNull());
}

class CheckAgeParam {
public:
    const char* pUrl;
    const char* pCacheDate;
    const char* pCacheLastModified;
    const char* pCacheExpires;
    long long cacheAge;
    bool cacheNoCacheExists;
    bool cacheMustRevalidateExists;
    const char* pCacheSentRequestTime;
    const char* pCacheReceivedResponseTime;
    long long cacheMaxAge;
    const char* pNow;
    long long requestMaxAge;
    long long requestMinFresh;
    long long requestMaxStale;
    bool getNetworkRequestExists;
    bool getCachedResponseExists;
    bool warning110Exists;
    bool warning113Exists;

    std::string print() const
    {
        std::string ret = std::string("\n") +
                "pUrl                        : " + (pUrl == NULL ? "" : pUrl) + "\n" +
                "pCacheDate                  : " + (pCacheDate == NULL ? "" : pCacheDate) + "\n" +
                "pCacheLastModified          : " + (pCacheLastModified == NULL ? "" : pCacheLastModified) + "\n" +
                "pCacheExpire                : " + (pCacheExpires == NULL ? "" : pCacheExpires) + "\n" +
                "cacheAge                    : " + Poco::NumberFormatter::format(static_cast<long>(cacheAge)) + "\n" +
                "cacheNoCacheExists          : " + StringUtil::boolToString(cacheNoCacheExists) + "\n" +
                "cacheMustRevalidateExists   : " + StringUtil::boolToString(cacheMustRevalidateExists) + "\n" + 
                "pCacheSentRequestTime       : " + (pCacheSentRequestTime == NULL ? "" : pCacheSentRequestTime) + "\n" +
                "pCacheReceivedResponseTime  : " +
                    (pCacheReceivedResponseTime == NULL ? "" : pCacheReceivedResponseTime) + "\n" +
                "cacheMaxAge                 : " + Poco::NumberFormatter::format(static_cast<long>(cacheMaxAge)) +
                "\n" +
                "now                         : " + (pNow == NULL ? "" : pNow) + "\n" +
                "requestMaxAge               : " + Poco::NumberFormatter::format(static_cast<long>(requestMaxAge)) +
                "\n" +
                "requestMinFresh             : " + Poco::NumberFormatter::format(static_cast<long>(requestMinFresh))
                + "\n" +
                "requestMaxStale             : " + Poco::NumberFormatter::format(static_cast<long>(requestMaxStale))
                + "\n" +
                "getNetworkRequestExists     : " + StringUtil::boolToString(getNetworkRequestExists) + "\n" +
                "getCachedResponseExists     : " + StringUtil::boolToString(getCachedResponseExists) + "\n" +
                "warning110Exists            : " + StringUtil::boolToString(warning110Exists) + "\n" +
                "warning113Exists            : " + StringUtil::boolToString(warning113Exists) + "\n";
        return ret;
    }
};
static const CheckAgeParam CheckAgeData[] = {
    {   // (0) Age がCache のmax-ageのはんいない (Cache Ageあり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:29:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (1) Age がCache のmax-ageより1秒少ない → はんいない (Cache Ageあり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:29:59 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (2) Age がCache のmax-ageと同じ → はんい外 (Cache Ageあり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (3) Age がCache のmax-ageのはんい外 (Cache Age あり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:01 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (4) Age がCache のmax-ageのはんい外 (Cache Ageあり, Cache に no-cache あり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        true,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:29:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (5) Age がCache のmax-ageのはんい外 (Cache Age あり, responeDurationでオーバー)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 11:49:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:20:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (6) Age がCache のmax-ageのはんいない
        //     (receivedResponseSec - Date 使用(Age より、receivedResponseSec - Date の方が大きい))
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:20:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:19:59 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (7) Age がCache のmax-ageのはんい外
        //     (receivedResponseSec - Date 使用(Age より、receivedResponseSec - Date の方が大きい)))
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:20:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:20:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (8) Age がCache のmax-ageのはんいない (receivedResponseSec - Date 使用, responseDuration がちょうどはんいない)
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:20:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 11:50:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:09:59 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (9) Age がCache のmax-ageのはんい外 (receivedResponseSec - Date 使用, responseDuration でオーバー)
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:20:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 11:50:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:10:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (10) Age がCache のExpiresのはんいない (Cache Ageあり, Date なし、Expires あり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 12:30:01 GMT", // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (11) Age がCache のExpiresのはんい外 (Cache Ageあり, Date なし、Expires あり)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 12:30:00 GMT", // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (12) Age がCache のExpiresのはんいない (Cache Ageあり, Date とExpires あり)
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:30:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 12:30:01 GMT", // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (13) Age がCache のExpiresのはんい外 (Cache Ageあり, Date とExpires あり)
        RequestUrl, // pUrl
        "Fri, 05 Aug 2016 11:30:00 GMT", // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 12:30:00 GMT", // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (14) Age がCache のLast-Modifiedからのintervalの10% はんいない (url にquery なし)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        "Fri, 05 Aug 2016 11:50:00 GMT", // pCacheLastModified
        NULL, // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:00:59 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (15) Age がCache のLast-Modifiedからのintervalの10% はんい外 (url にquery なし)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        "Fri, 05 Aug 2016 11:50:00 GMT", // pCacheLastModified
        NULL, // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:01:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (16) Age がCache のLast-Modifiedからのintervalの10% はんいない (url にquery あり)

        RequetUrlQuery, // pUrl
        NULL, // pCacheDate
        "Fri, 05 Aug 2016 11:50:00 GMT", // pCacheLastModified
        NULL, // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:00:59 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (17) Age がCache のLast-Modifiedからのintervalの10% はんいない (url にquery なし) Age が1日を超える
        RequestUrl, // pUrl
        NULL, // pCacheDate
        "Mon, 01 Aug 2016 12:00:00 GMT", // pCacheLastModified
        NULL, // pCacheExpires
        -1, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Mon, 15 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Mon, 15 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Tue, 16 Aug 2016 12:00:01 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        true    // warning113Exists
    },
    {   // (18) Age がCache のLast-Modifiedからのintervalの10% はんいない (url にquery なし) Age がちょうど1日
        RequestUrl, // pUrl
        NULL, // pCacheDate
        "Mon, 01 Aug 2016 12:00:00 GMT", // pCacheLastModified
        NULL, // pCacheExpires
        -1, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Mon, 15 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Mon, 15 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Tue, 16 Aug 2016 12:00:00 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (19) Age がRequest の max-ageの はんいない (Cache Response のmax-age より、Request の max-age が小さい)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        4000,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        3601, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (20) Age がRequest の max-ageの はんい外 (Cache Response のmax-age より、Request の max-age が小さい)

        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        4000,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        3600, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (21) Age がRequest の max-ageの はんいない (Cache Response のmax-age より、Request の max-age が大きい)
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        4000,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        3601, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (22) Age がExpire のはんいないだが、Request のmax-age のはんい外
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 13:00:00 GMT", // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        1799, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,  // getNetworkRequestExists
        false,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (23) Age がExpire のはんい外で、Request のmax-age のはんいない(Expires が使われる)

        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        "Fri, 05 Aug 2016 12:30:00 GMT", // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        -1,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        1801, // requestMaxAge
        -1, // requestMinFresh
        -1, // requestMaxStale
        true,  // getNetworkRequestExists
        false,   // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (24) Age が max-age + Request の max-stale のはんいない
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:09 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        10, // requestMaxStale
        false,  // getNetworkRequestExists
        true,   // getCachedResponseExists
        true,   // warning110Exists
        false   // warning113Exists
    },
    {   // (25) Age が max-age + Request の max-stale のはんい外
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:10 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        10, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (26) Age が max-age + Request の max-stale のはんいないだが、must-revalidate があるので、Age が max-age のはんい外
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        1800, // cacheAge
        false,  // cacheNoCacheExists
        true,   // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:09 GMT",    // pNow
        -1, // requestMaxAge
        -1, // requestMinFresh
        10, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (27) Age + Request の min-fresh が、max-age のはんいない
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        1799, // requestMinFresh
        -1, // requestMaxStale
        false,   // getNetworkRequestExists
        true,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
    {   // (28) Age + Request の min-fresh が、max-age のはんい外
        RequestUrl, // pUrl
        NULL, // pCacheDate
        NULL, // pCacheLastModified
        NULL, // pCacheExpires
        0, // cacheAge
        false,  // cacheNoCacheExists
        false,  // cacheMustRevalidateExists
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheSentRequestTime
        "Fri, 05 Aug 2016 12:00:00 GMT",    // pCacheReceivedResponseTime
        3600,   // cacheMaxAge
        "Fri, 05 Aug 2016 12:30:00 GMT",    // pNow
        -1, // requestMaxAge
        1800, // requestMinFresh
        -1, // requestMaxStale
        true,   // getNetworkRequestExists
        false,  // getCachedResponseExists
        false,  // warning110Exists
        false   // warning113Exists
    },
};

class CheckAgeTest : public HttpCacheStrategyUnitTest ,
public testing::WithParamInterface<CheckAgeParam> {
};
INSTANTIATE_TEST_CASE_P(HttpCacheStrategyUnitTest, CheckAgeTest,
        testing::ValuesIn(CheckAgeData));

TEST_P(CheckAgeTest, constructor_resolvesCacheStrategy_WhenByRequestAndCacheResponseParameter)
{
    CheckAgeParam& param = (CheckAgeParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given:
    Request::Builder cachedRequestBuilder;
    Request::Ptr pCachedRequest = cachedRequestBuilder.setUrl(param.pUrl).build();

    Response::Builder cachedResponseBuilder;
    if (param.pCacheDate != NULL) {
        cachedResponseBuilder.setHeader(HeaderDate, param.pCacheDate);
    }
    if (param.pCacheLastModified != NULL) {
        cachedResponseBuilder.setHeader(HeaderLastModified, param.pCacheLastModified);
    }
    if (param.pCacheExpires != NULL) {
        cachedResponseBuilder.setHeader(HeaderExpires, param.pCacheExpires);
    }
    if (param.cacheAge != -1) {
        cachedResponseBuilder.setHeader(HeaderAge, Poco::NumberFormatter::format(static_cast<long>(param.cacheAge)));
    }
    if (param.pCacheSentRequestTime != NULL) {
        Poco::Timestamp sentRequestTime;
        ASSERT_TRUE(HttpUtil::tryParseDate(param.pCacheSentRequestTime, sentRequestTime));
        cachedResponseBuilder.setSentRequestSec(sentRequestTime.epochTime());
    }
    if (param.pCacheReceivedResponseTime != NULL) {
        Poco::Timestamp receivedResponseTime;
        ASSERT_TRUE(HttpUtil::tryParseDate(param.pCacheReceivedResponseTime, receivedResponseTime));
        cachedResponseBuilder.setReceivedResponseSec(receivedResponseTime.epochTime());
    }
    if (param.cacheNoCacheExists || param.cacheMaxAge != -1 || param.cacheMustRevalidateExists) {
        CacheControl::Builder cachedCacheControlBuilder;
        if (param.cacheNoCacheExists) {
            cachedCacheControlBuilder.setNoCache(true);
        }
        if (param.cacheMaxAge != -1) {
            cachedCacheControlBuilder.setMaxAgeSec(param.cacheMaxAge);
        }
        if (param.cacheMustRevalidateExists) {
            cachedCacheControlBuilder.setMustRevalidate(param.cacheMustRevalidateExists);
        }
        cachedResponseBuilder.setCacheControl(cachedCacheControlBuilder.build());
    }
    Response::Ptr pInCachedResponse = cachedResponseBuilder.setRequest(pCachedRequest).build();

    Request::Builder requestBuilder;
    if (param.requestMaxAge != -1 || param.requestMinFresh != -1 || param.requestMaxStale != -1) {
        CacheControl::Builder requestCacheControlBuilder;
        if (param.requestMaxAge != -1) {
            requestCacheControlBuilder.setMaxAgeSec(param.requestMaxAge);
        }
        if (param.requestMinFresh != -1) {
            requestCacheControlBuilder.setMinFreshSec(param.requestMinFresh);
        }
        if (param.requestMaxStale != -1) {
            requestCacheControlBuilder.setMaxStaleSec(param.requestMaxStale);
        }
        requestBuilder.setCacheControl(requestCacheControlBuilder.build());
    }
    Request::Ptr pRequest = requestBuilder.setUrl(param.pUrl).build();

    Poco::Timestamp now;
    ASSERT_TRUE(HttpUtil::tryParseDate(param.pNow, now));
    unsigned long long nowAtEpoch = static_cast<unsigned long long>(now.epochTime());

    // When: create HttpCacheStrategy
    HttpCacheStrategy httpCacheStrategy(pRequest, pInCachedResponse, nowAtEpoch);

    // Then:
    // check NetworkRequest
    if (param.getNetworkRequestExists) {
        EXPECT_FALSE(httpCacheStrategy.getNetworkRequest().isNull());
    } else {
        EXPECT_TRUE(httpCacheStrategy.getNetworkRequest().isNull());
    }
    // check CachedResponse
    Response::Ptr pOutCachedResponse = httpCacheStrategy.getCachedResponse();
    if (param.getCachedResponseExists) {
        EXPECT_FALSE(pOutCachedResponse.isNull());
        if (param.warning110Exists) {
            EXPECT_TRUE(isExistedWarning(pOutCachedResponse, "110"));
        } else {
            EXPECT_TRUE(isNotExistedWarning(pOutCachedResponse, "110"));
        }
        if (param.warning113Exists) {
            EXPECT_TRUE(isExistedWarning(pOutCachedResponse, "113"));
        } else {
            EXPECT_TRUE(isNotExistedWarning(pOutCachedResponse, "113"));
        }
    } else {
        EXPECT_TRUE(pOutCachedResponse.isNull());
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
