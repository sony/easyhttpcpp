/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_CONTENTVALUES_H_INCLUDED
#define EASYHTTPCPP_DB_CONTENTVALUES_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Util/MapConfiguration.h"

namespace easyhttpcpp {
namespace db {

class ContentValues : public Poco::RefCountedObject {
public:
    ContentValues();
    virtual ~ContentValues();

    // TODO need unsigned value as well
    virtual void put(const std::string& key, int value);
    virtual void put(const std::string& key, unsigned int value);
    virtual void put(const std::string& key, float value);
    virtual void put(const std::string& key, short value);
    virtual void put(const std::string& key, double value);
    virtual void put(const std::string& key, long value);
    virtual void put(const std::string& key, unsigned long value);
    virtual void put(const std::string& key, long long value);
    virtual void put(const std::string& key, unsigned long long value);
    virtual void put(const std::string& key, const std::string& value);
    virtual void getKeys(std::vector<std::string>& keys) const;
    virtual std::string getStringValue(const std::string& key) const;
private:
    Poco::AutoPtr<Poco::Util::MapConfiguration> m_pConf;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_CONTENTVALUES_H_INCLUDED */
