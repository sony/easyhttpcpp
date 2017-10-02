/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEDATABASE_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEDATABASE_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/db/SqliteOpenHelper.h"

#include "HttpCacheMetadata.h"

namespace easyhttpcpp {

class HttpCacheEnumerationListener;

class HttpCacheDatabase : public easyhttpcpp::db::SqliteOpenHelper, public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpCacheDatabase> Ptr;

    HttpCacheDatabase(const Poco::Path& databaseFile);
    virtual ~HttpCacheDatabase();

    void onCreate(easyhttpcpp::db::SqliteDatabase& db);
    void onUpgrade(easyhttpcpp::db::SqliteDatabase& db, unsigned int oldVersion, unsigned int newVersion);

    bool getMetadata(const std::string& key, HttpCacheMetadata*& pHttpCacheMetadata);
    bool deleteMetadata(const std::string& key);
    bool updateMetadata(const std::string& key, HttpCacheMetadata* pHttpCacheMetadata);
    bool updateLastAccessedSec(const std::string& key);
    bool enumerate(HttpCacheEnumerationListener* pListener);

    // for test method.
    class HttpCacheMetadataAll : public HttpCacheMetadata {
    public:
        void setLastAccessedAtEpoch(std::time_t lastAccessedAtEpoch);
        std::time_t getLastAccessedAtEpoch() const;
    private:
        std::time_t m_lastAccessedAtEpoch;
    };
    bool getMetadataAll(const std::string& key, HttpCacheMetadataAll& httpCacheMetadataAll);
    bool updateMetadataAll(const std::string& key, HttpCacheMetadataAll& httpCacheMetadataAll);

private:
    HttpCacheDatabase();
    void dumpMetadata(HttpCacheMetadata* pHttpCacheMetadata);

    Poco::FastMutex m_mutex;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEDATABASE_H_INCLUDED */
