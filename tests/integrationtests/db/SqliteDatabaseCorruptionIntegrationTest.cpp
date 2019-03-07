/*
 * Copyright 2019 Sony Corporation
 */

#include <easyhttpcpp/db/AutoSqliteTransaction.h>
#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"
#include "EasyHttpCppAssertions.h"
#include "PartialMockSqliteOpenHelper.h"
#include "TestLogger.h"
#include "SqliteDatabaseIntegrationTestConstants.h"
#include "SqliteDatabaseTestUtil.h"

using namespace ::testing;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::PartialMockSqliteOpenHelper;

namespace easyhttpcpp {
namespace db {
namespace test {

namespace {
const std::string DatabaseDirString = SqliteDatabaseIntegrationTestConstants::DatabaseDir;
const std::string DatabaseFileName = SqliteDatabaseIntegrationTestConstants::DatabaseFileName;
const std::string DatabaseTableName = SqliteDatabaseIntegrationTestConstants::DatabaseTableName;
} /* namespace */

class SqliteDatabaseCorruptionIntegrationTest : public testing::Test {
protected:

    void SetUp()
    {
        Poco::File databaseDir(DatabaseDirString);
        FileUtil::createDirsIfAbsent(databaseDir);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    void TearDown()
    {
        EASYHTTPCPP_TESTLOG_TEARDOWN_START();

        m_pTestUtil = NULL;

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
        Poco::File databaseFile(databaseFilePath.absolute().toString());
        FileUtil::removeFileIfPresent(databaseFile);
    }

    void createInvalidDbFile() {
        Poco::File databaseFile(Poco::Path(DatabaseDirString, DatabaseFileName));

        // use a text file as a database file
        Poco::FileOutputStream fos(databaseFile.path());
        fos << "Hello, world!";
        fos.close();
    }

    void createDefaultDatabaseTableWithoutData()
    {
        // create table
        // table is as below
        // |Id | Name           | Address       | Age |
        std::string columnString = "Id INTEGER PRIMARY KEY AUTOINCREMENT, \
            Name VARCHAR(30) UNIQUE, Address VARCHAR, Age INTEGER(3)";

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);

        m_pTestUtil = new SqliteDatabaseTestUtil();
        m_pTestUtil->createAndOpenDatabase(databaseFilePath, 1);
        m_pTestUtil->createTable(DatabaseTableName, columnString);
    }

    void makeDatabaseCorrupted() {
        Poco::File databaseFile(Poco::Path(DatabaseDirString, DatabaseFileName));

        // replace start of database file with some garbage
        Poco::FileOutputStream fos(databaseFile.path());
        fos.seekp(0);
        fos << "Hello, world!";
        fos.close();
    }

    SqliteDatabaseTestUtil::Ptr m_pTestUtil;
};

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        getReadableDatabase_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsNotADbFile)
{
    // Given: database file is a text file
    createInvalidDbFile();

    // setup SqliteOpenHelper
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(0);

    // When: call getReadableDatabase
    // Then: throws SqlDatabaseCorruptionException
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(sqliteOpenHelper.getReadableDatabase(), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        getWritableDatabase_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsNotADbFile)
{
    // Given: database file is a text file
    createInvalidDbFile();

    // setup SqliteOpenHelper
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(0);

    // When: call getReadableDatabase
    // Then: throws SqlDatabaseCorruptionException
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(sqliteOpenHelper.getWritableDatabase(), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        execSql_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: execSql
    // Then: throws SqlDatabaseCorruptionException
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->execSql("PRAGMA user_version;"), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest, query_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: query
    // Then: throws SqlDatabaseCorruptionException
    std::vector<std::string> columns;
    columns.push_back("Id");
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false),
            SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        rawQuery_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: rawQuery
    // Then: throws SqlDatabaseCorruptionException
    std::vector<std::string> columns;
    columns.push_back("Id");
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    std::string queryString = SqliteQueryBuilder::buildQueryString(DatabaseTableName, &columns, NULL, NULL, NULL, NULL,
            NULL, false);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->rawQuery(queryString, NULL), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest, insert_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: insert data
    // Then: throws SqlDatabaseCorruptionException
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);

    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->insert(DatabaseTableName, *pContentValues), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        replace_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: replace data
    // Then: throws SqlDatabaseCorruptionException
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);

    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->replace(DatabaseTableName, *pContentValues), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        deleteRows_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: delete data
    // Then: throws SqlDatabaseCorruptionException
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10";

    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->deleteRows(DatabaseTableName, &whereClause, NULL), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        update_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database with corruption
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: update data
    // Then: throws SqlDatabaseCorruptionException
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    std::string whereClause = "Age = 10";
    AutoSqliteTransaction autoTransaction(pDb);

    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->update(DatabaseTableName, *(pContentValues.get()), &whereClause, NULL),
            SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        getVersion_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database and begin a transaction
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: getVersion
    // Then: throws SqlDatabaseCorruptionException
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(m_pTestUtil->getDatabase()->getVersion(), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        setVersion_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database and begin a transaction
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: setVersion
    // Then: throws SqlDatabaseCorruptionException
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(m_pTestUtil->getDatabase()->setVersion(1), SqlDatabaseCorruptException, 100203);
}

TEST_F(SqliteDatabaseCorruptionIntegrationTest,
        setAutoVacuum_ThrowsSqlDatabaseCorruptionException_WhenDatabaseFileIsCorrupted)
{
    // Given: init database and begin a transaction
    createDefaultDatabaseTableWithoutData();

    // introduce corruption to database file
    makeDatabaseCorrupted();

    // When: setAutoVacuum
    // Then: throws SqlDatabaseCorruptionException
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(m_pTestUtil->getDatabase()->setAutoVacuum(SqliteDatabase::AutoVacuumFull),
            SqlDatabaseCorruptException, 100203);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
