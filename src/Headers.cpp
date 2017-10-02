/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/String.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpException.h"

namespace easyhttpcpp {

static const std::string Tag = "Headers";

Headers::Headers()
{
}

Headers::Headers(const Headers& original)
{
    m_headerItems = original.m_headerItems;
}

Headers::~Headers()
{
}

Headers& Headers::Headers::operator = (const Headers& original)
{
    if (&original != this) {
        m_headerItems = original.m_headerItems;
    }
    return *this;
}

void Headers::add(const std::string& name, const std::string& value)
{
    if (name.empty()) {
        std::string message = "can not add empty name in header.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }

    m_headerItems.insert(HeaderMap::ValueType(name, value));
}

void Headers::set(const std::string& name, const std::string& value)
{
    if (name.empty()) {
        std::string message = "can not set empty name in header.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpIllegalArgumentException(message);
    }

    m_headerItems.erase(name);

    // use the Poco::ListMap with caution.
    // ListMap::add method make same name.
    // ListMap::find get first found name.
    m_headerItems[name] = value;
}

const std::string& Headers::getValue(const std::string& name, const std::string& defaultValue) const
{
    HeaderMap::ConstIterator it = m_headerItems.find(name);
    if (it == m_headerItems.end()) {
        return defaultValue;
    } else {
        return it->second;
    }
}

bool Headers::has(const std::string& name) const
{
    HeaderMap::ConstIterator it = m_headerItems.find(name);
    if (it == m_headerItems.end()) {
        return false;
    } else {
        return true;
    }
}

size_t Headers::getSize() const
{
    return m_headerItems.size();
}

bool Headers::empty() const
{
    return m_headerItems.empty();
}

std::string Headers::toString()
{
    std::string str;
    for (HeaderMap::ConstIterator it = m_headerItems.begin(); it != m_headerItems.end(); it++) {
        str += it->first + ":" + it->second + "\n";
    }
    return str;
}

Headers::HeaderMap::ConstIterator Headers::begin() const
{
    return m_headerItems.begin();
}

Headers::HeaderMap::ConstIterator Headers::end() const
{
    return m_headerItems.end();
}

} /* namespace easyhttpcpp */
