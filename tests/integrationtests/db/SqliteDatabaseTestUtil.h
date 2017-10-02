/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_TEST_INTEGRATIONTEST_SQLITEDATABASETESTUTIL_H_INCLUDED
#define EASYHTTPCPP_DB_TEST_INTEGRATIONTEST_SQLITEDATABASETESTUTIL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Path.h"

#include "easyhttpcpp/db/SqliteCursor.h"
#include "easyhttpcpp/db/SqliteDatabase.h"
#include "PartialMockSqliteOpenHelper.h"

namespace easyhttpcpp {
namespace db {
namespace test {

class SqliteDatabaseTestUtil : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<SqliteDatabaseTestUtil> Ptr;

    SqliteDatabaseTestUtil();
    virtual ~SqliteDatabaseTestUtil();

    bool createAndOpenDatabase(const Poco::Path& path, unsigned int version);
    bool createTable(const std::string tableName, const std::string& column);
    bool insertData(const std::string tableName, std::vector<ContentValues*>& values);
    SqliteDatabase::Ptr getDatabase();
    SqliteCursor::Ptr queryDatabase(const std::string& table, const std::vector<std::string>* columns,
            const std::string* selection, const std::vector<std::string>* selectionArgs);
    void clearDatabaseFiles(const Poco::Path& path);

private:
    easyhttpcpp::testutil::PartialMockSqliteOpenHelper::Ptr m_pHelper;
    SqliteDatabase::Ptr m_pDatabase;
};

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_TEST_INTEGRATIONTEST_SQLITEDATABASETESTUTIL_H_INCLUDED */
