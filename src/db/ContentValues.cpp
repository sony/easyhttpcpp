/*
 * Copyright 2017 Sony Corporation
 */

#include <sstream>

#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

static const std::string Tag = "ContentValues";

ContentValues::ContentValues()
{
    m_pConf = new Poco::Util::MapConfiguration;
}

ContentValues::~ContentValues()
{
}

void ContentValues::put(const std::string& key, int value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, unsigned int value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, float value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, short value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, double value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, long value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, unsigned long value)
{
    put(key, Poco::NumberFormatter::format(value));
}

void ContentValues::put(const std::string& key, long long value)
{
    std::stringstream ss;
    ss << value;
    put(key, ss.str());
}

void ContentValues::put(const std::string& key, unsigned long long value)
{
    std::stringstream ss;
    ss << value;
    put(key, ss.str());
}

void ContentValues::put(const std::string& key, const std::string& value)
{
    m_pConf->setString(key, value);
}

void ContentValues::getKeys(std::vector<std::string>& keys) const
{
    m_pConf->keys(keys);
}

std::string ContentValues::getStringValue(const std::string& key) const
{
    try {
        return m_pConf->getString(key);
    } catch (const Poco::Exception& e) {
        std::string msg = "getStringValue invalid key : " + key;
        EASYHTTPCPP_LOG_D(Tag, msg.c_str());
        throw SqlIllegalArgumentException(msg, e);
    }
}

} /* namespace db */
} /* namespace easyhttpcpp */
