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
#include "easyhttpcpp/HttpExports.h"

#include "HttpCacheDatabaseOpenHelper.h"
#include "HttpCacheMetadata.h"

namespace easyhttpcpp {

class HttpCacheEnumerationListener;

class EASYHTTPCPP_HTTP_INTERNAL_API HttpCacheDatabase : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpCacheDatabase> Ptr;

    HttpCacheDatabase(HttpCacheDatabaseOpenHelper::Ptr pOpenHelper);
    virtual ~HttpCacheDatabase();

    HttpCacheMetadata::Ptr getMetadata(const std::string& key);
    bool deleteMetadata(const std::string& key);
    void updateMetadata(const std::string& key, HttpCacheMetadata::Ptr pHttpCacheMetadata);
    bool updateLastAccessedSec(const std::string& key);
    bool deleteDatabaseFile();
    void enumerate(HttpCacheEnumerationListener* pListener);
    void closeSqliteSession();

    // for test method.
    class EASYHTTPCPP_HTTP_INTERNAL_API HttpCacheMetadataAll : public HttpCacheMetadata {
    public:
        typedef Poco::AutoPtr<HttpCacheMetadataAll> Ptr;

        HttpCacheMetadataAll();
        void setLastAccessedAtEpoch(std::time_t lastAccessedAtEpoch);
        std::time_t getLastAccessedAtEpoch() const;
    private:
        std::time_t m_lastAccessedAtEpoch;
    };
    HttpCacheMetadataAll::Ptr getMetadataAll(const std::string& key);
    void updateMetadataAll(const std::string& key, HttpCacheMetadataAll::Ptr pHttpCacheMetadataAll);

private:
    HttpCacheDatabase();
    void dumpMetadata(HttpCacheMetadata::Ptr pHttpCacheMetadata);

    Poco::FastMutex m_mutex;
    HttpCacheDatabaseOpenHelper::Ptr m_pOpenHelper;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEDATABASE_H_INCLUDED */
