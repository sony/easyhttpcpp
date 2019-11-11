/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITEOPENHELPER_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITEOPENHELPER_H_INCLUDED

#include <string>

#include "Poco/Mutex.h"
#include "Poco/Path.h"

#include "easyhttpcpp/db/DbExports.h"
#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqliteDatabaseCorruptionListener.h"

namespace easyhttpcpp {
namespace db {

class EASYHTTPCPP_DB_API SqliteOpenHelper {
public:
    SqliteOpenHelper(const Poco::Path& path, unsigned int version);
    virtual ~SqliteOpenHelper();

    virtual const Poco::Path& getDatabasePath() const;
    virtual unsigned int getDatabaseVersion() const;

    virtual SqliteDatabase::Ptr getWritableDatabase();
    virtual SqliteDatabase::Ptr getReadableDatabase();
    virtual void onCreate(SqliteDatabase& db) = 0;
    virtual void onConfigure(SqliteDatabase& db);
    virtual void onOpen(SqliteDatabase& db);
    virtual void onUpgrade(SqliteDatabase& db, unsigned int oldVersion, unsigned int newVersion) = 0;
    virtual void onDowngrade(SqliteDatabase& db, unsigned int oldVersion, unsigned int newVersion);
    virtual void close();
    virtual void closeSqliteSession();
    virtual void setDatabaseCorruptionListener(SqliteDatabaseCorruptionListener::Ptr pListener);

    // this method is for internal testing only.
    virtual void overrideInternalDatabase(SqliteDatabase::Ptr pDatabase);

private:
    SqliteDatabase::Ptr getDatabase();
    SqliteDatabase::Ptr createDatabase();

    bool m_initializing;
    Poco::Path m_path;
    unsigned int m_version;
    SqliteDatabase::Ptr m_database;
    SqliteDatabaseCorruptionListener::Ptr m_pDatabaseCorruptionListener;

    Poco::FastMutex m_mutex;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITEOPENHELPER_H_INCLUDED */
