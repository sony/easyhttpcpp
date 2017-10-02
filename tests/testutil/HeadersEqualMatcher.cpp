/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>
#include <Poco/String.h>
#include "Poco/JSON/Object.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "HeadersEqualMatcher.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::Headers;

namespace easyhttpcpp {
namespace testutil {

static std::string exchangeHeadersToJsonStr(Headers::Ptr pHeaders)
{
    Poco::JSON::Object headerJson;
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        headerJson.set(it->first, it->second);
    }
    std::stringstream strStream;
    headerJson.stringify(strStream);
    return strStream.str();
}

HeadersEqualMatcher::HeadersEqualMatcher(easyhttpcpp::Headers::Ptr pHeaders) : m_pExpectedHeaders(pHeaders)
{
}

HeadersEqualMatcher::~HeadersEqualMatcher()
{
}

bool HeadersEqualMatcher::MatchAndExplain(easyhttpcpp::Headers::Ptr pActual,
        testing::MatchResultListener* listener) const
{
    if (!pActual) {
        if (!m_pExpectedHeaders) {
            return true;
        }
        *listener << "actual headers is NULL";
        return false;
    }
    if (!m_pExpectedHeaders) {
        *listener << "expected header is NULL";
        return false;
    }
    if (pActual->getSize() != m_pExpectedHeaders->getSize()) {
        *listener <<
                StringUtil::format("different size. expected:%zu actual:%zu headers(json):%s",
                m_pExpectedHeaders->getSize(), pActual->getSize(), exchangeHeadersToJsonStr(pActual).c_str());
        return false;
    }
    for (Headers::HeaderMap::ConstIterator it = pActual->begin(); it != pActual->end(); it++) {
        if (!m_pExpectedHeaders->has(it->first)) {
            *listener << StringUtil::format("\"%s\" does not found in expected headers. actual headers(json):%s",
                    it->first.c_str(), exchangeHeadersToJsonStr(pActual).c_str());
            return false;
        }
        const std::string& expectedValue = m_pExpectedHeaders->getValue(it->first, "");
        if (Poco::icompare(it->second, expectedValue) != 0) {
            *listener <<
                    StringUtil::format("\"%s\" is different value. expected:\"%s\" actual:\"%s\" headers(json):%s",
                    it->first.c_str(), expectedValue.c_str(), it->second.c_str(),
                    exchangeHeadersToJsonStr(pActual).c_str());
            return false;
        }
    }
    return true;
}

void HeadersEqualMatcher::DescribeTo(std::ostream* os) const
{
    *os << "is equal to ";
    if (!m_pExpectedHeaders) {
        *os << exchangeHeadersToJsonStr(m_pExpectedHeaders);
    }
}

void HeadersEqualMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "is not equal to ";
    if (!m_pExpectedHeaders) {
        *os << exchangeHeadersToJsonStr(m_pExpectedHeaders);
    }
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
