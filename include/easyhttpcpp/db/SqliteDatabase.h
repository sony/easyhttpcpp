/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITEDATABASE_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITEDATABASE_H_INCLUDED

#include <string>
#include <vector>

#include "Poco/AutoPtr.h"
#include "Poco/Exception.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/db/DbExports.h"
#include "easyhttpcpp/db/SqliteCursor.h"
#include "easyhttpcpp/db/SqliteDatabaseCorruptionListener.h"

namespace easyhttpcpp {
namespace db {

class ContentValues;

class EASYHTTPCPP_DB_API SqliteDatabase : public Poco::RefCountedObject {
public:
    typedef long long RowId;
    typedef Poco::AutoPtr<SqliteDatabase> Ptr;

    enum AutoVacuum {
        // 0 means disable auto vacuum
        AutoVacuumNone,
        // 1 means enable full auto vacuum
        AutoVacuumFull,
        // 2 means enable incremental vacuum
        AutoVacuumIncremental
    };

    SqliteDatabase(const std::string& path);

    virtual ~SqliteDatabase();

    static SqliteDatabase::Ptr openOrCreateDatabase(const std::string& path);

    virtual void execSql(const std::string& sql);

    virtual SqliteCursor::Ptr query(const std::string& table, const std::vector<std::string>* columns,
            const std::string* selection, const std::vector<std::string>* selectionArgs, const std::string* groupBy,
            const std::string* having, const std::string* orderBy, const std::string* limit,
            const bool distinct = true);

    virtual SqliteCursor::Ptr rawQuery(const std::string& sql, const std::vector<std::string>* selectionArgs);

    virtual void insert(const std::string& table, const ContentValues& values);

    virtual void replace(const std::string& table, const ContentValues& initialValues);

    virtual size_t deleteRows(const std::string& table, const std::string* whereClause,
            const std::vector<std::string>* whereArgs);

    virtual size_t update(const std::string& table, const ContentValues& values,
            const std::string* whereClause, const std::vector<std::string>* whereArgs);

    virtual void beginTransaction();

    virtual void endTransaction();

    virtual void setTransactionSuccessful();

    virtual void close();

    virtual void closeSqliteSession();

    virtual unsigned int getVersion();

    virtual void setVersion(unsigned int version);

    virtual AutoVacuum getAutoVacuum();

    virtual void setAutoVacuum(AutoVacuum autoVacuum);

    virtual bool isOpen();

    virtual void reopen();

    virtual void setDatabaseCorruptionListener(SqliteDatabaseCorruptionListener::Ptr pListener);

private:
    Poco::SharedPtr<Poco::Data::Session> m_pSession;
    Poco::FastMutex m_mutex;
    SqliteDatabaseCorruptionListener::Ptr m_pDatabaseCorruptionListener;
    std::string m_path;

    bool m_opened;

    void throwExceptionIfIllegalState();
    void checkForDatabaseCorruption(const std::string& funcName, const Poco::Exception& exception);
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITEDATABASE_H_INCLUDED */
