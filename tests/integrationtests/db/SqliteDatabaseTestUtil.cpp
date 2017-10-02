/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/File.h"
#include "Poco/Path.h"

#include "SqliteDatabaseTestUtil.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/AutoSqliteTransaction.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::db::AutoSqliteTransaction;
using easyhttpcpp::db::SqlException;
using easyhttpcpp::db::SqliteQueryBuilder;
using easyhttpcpp::testutil::PartialMockSqliteOpenHelper;

namespace easyhttpcpp {
namespace db {
namespace test {

static const std::string Tag = "SqliteDatabaseTestUtil";

SqliteDatabaseTestUtil::SqliteDatabaseTestUtil()
{
}

SqliteDatabaseTestUtil::~SqliteDatabaseTestUtil()
{
}

bool SqliteDatabaseTestUtil::createAndOpenDatabase(const Poco::Path& path, unsigned int version)
{
    bool ret = false;
    m_pHelper = new PartialMockSqliteOpenHelper(path, version);
    EXPECT_CALL(*m_pHelper, onCreate(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*m_pHelper, onConfigure(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*m_pHelper, onOpen(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*m_pHelper, onUpgrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*m_pHelper, onDowngrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());

    try {
        m_pDatabase = m_pHelper->getWritableDatabase();
        ret = true;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "catch exception in createAndOpenDatabase [%s]", e.getMessage().c_str());
        e.rethrow();
    }
    return ret;
}

bool SqliteDatabaseTestUtil::createTable(const std::string tableName, const std::string& column)
{
    bool ret = false;

    try {
        // create table and insert initial data
        AutoSqliteTransaction autoTransaction(m_pDatabase);

        std::string sql = StringUtil::format("DROP TABLE IF EXISTS %s", tableName.c_str());
        m_pDatabase->execSql(sql);

        sql = StringUtil::format("CREATE TABLE %s (%s)", tableName.c_str(), column.c_str());
        m_pDatabase->execSql(sql);

        m_pDatabase->setTransactionSuccessful();
        ret = true;

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "catch exception in createAndOpenDatabase [%s]", e.getMessage().c_str());
        e.rethrow();
    }
    return ret;
}

bool SqliteDatabaseTestUtil::insertData(const std::string tableName, std::vector<ContentValues*>& values)
{
    bool ret = false;

    try {
        // create table and insert initial data
        AutoSqliteTransaction autoTransaction(m_pDatabase);

        //
        size_t count = values.size();
        if (values.size() != 0) {
            for (size_t i = 0; i < count; i++) {
                ContentValues* pContentValue = values.at(i);
                std::string sql = SqliteQueryBuilder::buildInsertString(tableName, *pContentValue,
                        easyhttpcpp::db::SqliteConflictAlgorithmNone);
                m_pDatabase->execSql(sql);
            }
        }

        m_pDatabase->setTransactionSuccessful();
        ret = true;

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "catch exception in createAndOpenDatabase [%s]", e.getMessage().c_str());
        e.rethrow();
    }
    return ret;
}

SqliteDatabase::Ptr SqliteDatabaseTestUtil::getDatabase()
{
    return m_pDatabase;
}

SqliteCursor::Ptr SqliteDatabaseTestUtil::queryDatabase(const std::string& table,
        const std::vector<std::string>* columns, const std::string* selection,
        const std::vector<std::string>* selectionArgs)
{
    try {
        SqliteCursor::Ptr pCursor = m_pDatabase->query(table, columns, selection, selectionArgs, NULL, NULL, NULL,
                NULL, false);
        return pCursor;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "catch exception in queryDatabase");
        return NULL;
    }
}

void SqliteDatabaseTestUtil::clearDatabaseFiles(const Poco::Path& path)
{
    Poco::File databaseFile(path);
    FileUtil::removeFileIfPresent(databaseFile);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
