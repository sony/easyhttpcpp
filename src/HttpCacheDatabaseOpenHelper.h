/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEDATABASEOPENHELPER_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEDATABASEOPENHELPER_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Path.h"

#include "easyhttpcpp/db/SqliteOpenHelper.h"
#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API HttpCacheDatabaseOpenHelper : public easyhttpcpp::db::SqliteOpenHelper,
public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpCacheDatabaseOpenHelper> Ptr;

    HttpCacheDatabaseOpenHelper(const Poco::Path& databaseFile);
    virtual ~HttpCacheDatabaseOpenHelper();

    void onCreate(easyhttpcpp::db::SqliteDatabase& db);
    void onUpgrade(easyhttpcpp::db::SqliteDatabase& db, unsigned int oldVersion, unsigned int newVersion);
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEDATABASEOPENHELPER_H_INCLUDED */
