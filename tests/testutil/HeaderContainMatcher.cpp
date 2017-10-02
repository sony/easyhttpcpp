/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/StringUtil.h"
#include "HeaderContainMatcher.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::Headers;

namespace easyhttpcpp {
namespace testutil {

HeaderContainMatcher::HeaderContainMatcher(const std::string& name, const std::string& value) : m_name(name),
        m_value(value), m_keyOnly(false)
{
}

HeaderContainMatcher::HeaderContainMatcher(const std::string& name) : m_name(name), m_keyOnly(true)
{
}

HeaderContainMatcher::~HeaderContainMatcher()
{
}

bool HeaderContainMatcher::MatchAndExplain(easyhttpcpp::Headers::Ptr pActual,
        testing::MatchResultListener* listener) const
{
    if (!pActual) {
        *listener << "actual headers is NULL";
        return false;
    }
    if (m_name.empty()) {
        *listener << "expected name is empty";
        return false;
    }
    if (!pActual->has(m_name)) {
        *listener << StringUtil::format("\"%s\" does not found in headers", m_name.c_str());
        return false;
    }
    if (m_keyOnly) {
        return true;
    }
    if (Poco::icompare(pActual->getValue(m_name, ""), m_value) != 0) {
        *listener << StringUtil::format("\"%s\" and \"%s\" is different", pActual->getValue(m_name, "").c_str(),
                m_value.c_str());
        return false;
    }
    return true;
}

void HeaderContainMatcher::DescribeTo(std::ostream* os) const
{
    if (m_keyOnly) {
        *os << StringUtil::format("is have to \"%s\"", m_name.c_str());
    } else {
        *os << StringUtil::format("is contain to \"%s\":\"%s\"", m_name.c_str(), m_value.c_str());
    }
}

void HeaderContainMatcher::DescribeNegationTo(std::ostream* os) const
{
    if (m_keyOnly) {
        *os << StringUtil::format("is not have to \"%s\"", m_name.c_str());
    } else {
        *os << StringUtil::format("is not contain to \"%s\":\"%s\"", m_name.c_str(), m_value.c_str());
    }
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
