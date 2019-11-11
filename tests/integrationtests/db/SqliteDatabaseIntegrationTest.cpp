/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include <stdint.h>

#include "Poco/Path.h"
#include "Poco/Data/SQLite/Connector.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/AutoSqliteTransaction.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"
#include "EasyHttpCppAssertions.h"
#include "PartialMockSqliteOpenHelper.h"
#include "TestLogger.h"
#include "TestPreferences.h"

#include "SqliteDatabaseIntegrationTestConstants.h"
#include "SqliteDatabaseTestUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::PartialMockSqliteOpenHelper;
using easyhttpcpp::testutil::TestPreferences;

namespace easyhttpcpp {
namespace db {
namespace test {

namespace {
const std::string DatabaseDirString =
        FileUtil::convertToAbsolutePathString(SqliteDatabaseIntegrationTestConstants::DatabaseDir);
const std::string DatabaseFileName = SqliteDatabaseIntegrationTestConstants::DatabaseFileName;
const std::string DatabaseTableName = SqliteDatabaseIntegrationTestConstants::DatabaseTableName;
} /* namespace */

#define ELEMENT_NUM(st) (sizeof(st) / (sizeof st[0]))

class SqliteDatabaseIntegrationTest : public testing::Test {
protected:

    static void SetUpTestCase()
    {
        // initialize test preferences with QA profile
        TestPreferences::getInstance().initialize(TestPreferences::ProfileQA);
        Poco::Path databaseDir(DatabaseDirString);
        FileUtil::createDirsIfAbsent(Poco::File(databaseDir));
    }

    void TearDown()
    {
        EASYHTTPCPP_TESTLOG_TEARDOWN_START();

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
        m_pTestUtil = NULL;
        FileUtil::removeFileIfPresent(databaseFilePath);
    }

    void createDefaultDatabaseTable()
    {
        // create table and insert data
        // table is as below
        // |Id | Name           | Address       | Age |
        // | 1 | "Bart Simpson" | "Springfield" | 12  |
        // | 2 | "Lisa Simpson" | "Springfield" | 10  |
        //
        std::string columnString = "Id INTEGER PRIMARY KEY AUTOINCREMENT, \
            Name VARCHAR(30) UNIQUE, Address VARCHAR, Age INTEGER(3)";

        Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
        pContentValues->put("Name", "Bart Simpson");
        pContentValues->put("Address", "Springfield");
        pContentValues->put("Age", 12);


        Poco::AutoPtr<ContentValues> pContentValues2 = new ContentValues();
        pContentValues2->put("Name", "Lisa Simpson");
        pContentValues2->put("Address", "Springfield");
        pContentValues2->put("Age", 10);

        //push back pContentValues and pContentValues2 to valuesVec
        std::vector<ContentValues*> valuesVec;
        valuesVec.push_back(pContentValues);
        valuesVec.push_back(pContentValues2);

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);

        m_pTestUtil = new SqliteDatabaseTestUtil();
        m_pTestUtil->createAndOpenDatabase(databaseFilePath, 1);
        m_pTestUtil->createTable(DatabaseTableName, columnString);
        m_pTestUtil->insertData(DatabaseTableName, valuesVec);
    }

    void createDefaultDatabaseTableWithoutData()
    {
        // create table and insert data
        // table is as below
        std::string columnString = "Id INTEGER PRIMARY KEY AUTOINCREMENT, \
            Name VARCHAR(30) UNIQUE, Address VARCHAR, Age INTEGER(3)";

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);

        m_pTestUtil = new SqliteDatabaseTestUtil();
        m_pTestUtil->createAndOpenDatabase(databaseFilePath, 1);
        m_pTestUtil->createTable(DatabaseTableName, columnString);
    }

    SqliteCursor::Ptr queryDatabase(const std::string& tableName, const std::vector<std::string>* columns)
    {
        return m_pTestUtil->queryDatabase(tableName, columns, NULL, NULL);
    }

    struct RowElement {
        int m_id;
        char m_name[20];
        char m_address[20];
        int m_age;
    };

    bool databaseHasColumns(SqliteCursor::Ptr pCursor, size_t expectedColumnCount,
            std::vector<std::string>& expectedColumnNames)
    {
        bool ret = false;
        if (pCursor->getColumnCount() != expectedColumnCount) {
            return ret;
        }

        std::vector<std::string> columnNames = pCursor->getColumnNames();
        for (size_t i = 0; i < expectedColumnCount; i++) {
            if (columnNames.at(i) != expectedColumnNames.at(i)) {
                break;
            }
            ret = true;
        }
        return ret;
    }

    bool databaseHasRow(SqliteCursor::Ptr pCursor, RowElement& expectedRowElement)
    {
        bool ret = false;

        size_t idIndex = SIZE_MAX;
        size_t ageIndex = SIZE_MAX;
        size_t nameIndex = SIZE_MAX;
        size_t addressIndex = SIZE_MAX;

        // column count
        size_t columnCount = pCursor->getColumnCount();

        // column names
        std::vector<std::string> columnNames = pCursor->getColumnNames();

        for (size_t i = 0; i < columnCount; i++) {
            if (columnNames.at(i) == "Id") {
                idIndex = i;
            } else if (columnNames.at(i) == "Age") {
                ageIndex = i;
            } else if (columnNames.at(i) == "Name") {
                nameIndex = i;
            } else if (columnNames.at(i) == "Address") {
                addressIndex = i;
            }
        }

        if (idIndex == SIZE_MAX && ageIndex == SIZE_MAX && nameIndex == SIZE_MAX && addressIndex == SIZE_MAX) {
            return ret;
        }

        if (!pCursor->moveToFirst()) {
            return ret;
        }

        bool idExist = false;
        bool ageExist = false;
        bool nameExist = false;
        bool addressExist = false;
        do {
            idExist = false;
            ageExist = false;
            nameExist = false;
            addressExist = false;
            if (idIndex == SIZE_MAX || pCursor->getInt(idIndex) == expectedRowElement.m_id) {
                idExist = true;
            }

            if (ageIndex == SIZE_MAX || pCursor->getInt(ageIndex) == expectedRowElement.m_age) {
                ageExist = true;
            }

            if (nameIndex == SIZE_MAX || pCursor->getString(nameIndex) == expectedRowElement.m_name) {
                nameExist = true;
            }

            if (addressIndex == SIZE_MAX || pCursor->getString(addressIndex) == expectedRowElement.m_address) {
                addressExist = true;
            }

            if (idExist & ageExist & nameExist & addressExist) {
                ret = true;
                break;
            }
        } while (pCursor->moveToNext());

        return ret;
    }

    SqliteDatabaseTestUtil::Ptr m_pTestUtil;
};

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsQueryResult)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: query Age, and Name
    std::vector<std::string> columns;
    columns.push_back("Id");
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    SqliteCursor::Ptr pCursor = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, rawQuery_ReturnsQueryResult)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: buidQueryString and call rawQuery
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    std::string queryString = SqliteQueryBuilder::buildQueryString(DatabaseTableName, &columns, NULL, NULL, NULL, NULL,
            NULL, false);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    SqliteCursor::Ptr pCursor = pDb->rawQuery(queryString, NULL);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, insert_Succeeds)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->insert(DatabaseTableName, *pContentValues);
    pDb->setTransactionSuccessful();

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));
    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Homer Simpson", "Springfield", 38}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, replace_Succeeds_WhenPrimaryKeyConflicts)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 99);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->replace(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {3, "Lisa Simpson", "Springfield", 99},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, replace_Succeeds_WhenPrimaryKeyDoesNotConflict)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->replace(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Homer Simpson", "Springfield", 38},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, delete_Succeeds_WhenWhereClauseMatches)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: delete data
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10";
    size_t id = pDb->deleteRows(DatabaseTableName, &whereClause, NULL);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(1, id);

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, delete_DoNothing_WhenWhereClauseDoesNotMatch)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: delete data
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 33";
    size_t id = pDb->deleteRows(DatabaseTableName, &whereClause, NULL);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(0, id);

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, delete_Succeeds_WhenWhereClauseMatchesToMultiRaw)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Nobi Nobita");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 10);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // When: delete data
    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10";
    size_t id = pDb->deleteRows(DatabaseTableName, &whereClause, NULL);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(2, id);

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, update_Succeeds_WhenWhereClauseMatches)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: update data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Maggie Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 1);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10";
    size_t id = pDb->update(DatabaseTableName, *(pContentValues.get()), &whereClause, NULL);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(1, id);

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Maggie Simpson", "Springfield", 1}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, update_Succeeds_WhenWhereClauseHasMultiConditions)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Nobi Nobita");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 10);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // When: update data
    pContentValues->put("Name", "Maggie Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 1);

    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10 AND Address = 'Tokyo'";
    size_t id = pDb->update(DatabaseTableName, *(pContentValues.get()), &whereClause, NULL);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(1, id);

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Maggie Simpson", "Springfield", 1}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {3, "Nobi Nobita", "Tokyo", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, update_throwsSqlExecutionException_WhenWhereClauseMatchesToMultiRaw)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Nobi Nobita");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 10);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // When: update data
    pContentValues->put("Name", "Maggie Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 1);

    AutoSqliteTransaction autoTransaction(pDb);
    std::string whereClause = "Age = 10";
    size_t id = 0;
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(id = pDb->update(DatabaseTableName, *(pContentValues.get()), &whereClause, NULL),
            SqlExecutionException, 100202);
    pDb->setTransactionSuccessful();

    EXPECT_EQ(0, id);

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Nobi Nobita", "Tokyo", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Maggie Simpson", "Springfield", 1},
        {3, "Maggie Simpson", "Springfield", 1}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, beginTransaction_throwsSqlIllegalStateException_AfterCloseDatabase)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: close database
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->close();

    // Then: Access to SqliteDatabase throws Exception
    EASYHTTPCPP_EXPECT_THROW(pDb->beginTransaction(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteDatabaseIntegrationTest, query_Succeeds_WhenReopenDatabase)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: close SqliteDatabase
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->close();
    EASYHTTPCPP_EXPECT_THROW(pDb->beginTransaction(), SqlIllegalStateException, 100201);

    // When: reopen and insert data
    pDb->reopen();

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, endTransaction_RollbacksData_WhenWithoutCallSetTransactionSuccessful)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));

    // call SqliteDatabase::endTransaction() without call setTransactionSuccessful()
    pDb->endTransaction();

    // Then: verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {3, "Homer Simpson", "Springfield", 38}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsQueryResult_WhenSelectionArgsExists)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: query Age, and Name, with SelectionArgs "Age = 10"
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");

    std::string selection = "Age = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("10");

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    SqliteCursor::Ptr pCursor = pDb->query(DatabaseTableName, &columns, &selection, &selectionArgs, NULL, NULL, NULL,
            NULL, false);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsQueryResult_WhenSelectionArgsExistsMulti)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *pContentValues);
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // When: query Age, and Name, with SelectionArgs "Age = 10 OR Age = 38"
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");

    std::string selection = "Age = ? OR Age = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("10");
    selectionArgs.push_back("38");

    SqliteCursor::Ptr pCursor = pDb->query(DatabaseTableName, &columns, &selection, &selectionArgs, NULL, NULL, NULL,
            NULL, false);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Homer Simpson", "Springfield", 38}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, rawQuery_ReturnsQueryResult_WhenSelectionArgsExists)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: buidQueryString and 
    // call rawQuery with selectionArgs "Age = 10"
    std::string where = "Age = ?";

    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");

    std::string queryString = SqliteQueryBuilder::buildQueryString(DatabaseTableName, &columns, &where, NULL, NULL,
            NULL, NULL, false);

    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("10");
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    SqliteCursor::Ptr pCursor = pDb->rawQuery(queryString, &selectionArgs);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, rawQuery_ReturnsQueryResult_WhenSelectionArgsExistsMulti)
{
    // Given: create database and inset data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();

    // When: buidQueryString and 
    // call rawQuery with selectionArgs "Age = 10 OR Age = 38"
    std::string where = "Age = ? OR Age = ?";

    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");

    std::string queryString = SqliteQueryBuilder::buildQueryString(DatabaseTableName, &columns, &where, NULL, NULL,
            NULL, NULL, false);

    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("10");
    selectionArgs.push_back("38");
    SqliteCursor::Ptr pCursor = pDb->rawQuery(queryString, &selectionArgs);

    // Then: verify query result
    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Homer Simpson", "Springfield", 38}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsOnlyDistinctValues_WhenDistinctIsTrue)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Nobi Nobita");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 10);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // call query with distinct false, and verify query result row count is 3
    std::vector<std::string> columns;
    columns.push_back("Age");

    SqliteCursor::Ptr pCursor1 = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false);

    EXPECT_EQ(3, pCursor1->getCount());

    RowElement expectedRows1[] = {
        {1, "", "", 12},
        {2, "", "", 10},
        {3, "", "", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedRows1); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor1, expectedRows1[i]));
    }

    // When: call query with distinct true
    SqliteCursor::Ptr pCursor2 = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, true);

    // Then: verify query result, duplicate value should be removed
    EXPECT_EQ(2, pCursor2->getCount());

    RowElement expectedRows2[] = {
        {1, "", "", 12},
        {2, "", "", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedRows2); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor2, expectedRows2[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsAllMatchingValues_WhenDistinctIsTrue)
{
    // Given: create database and insert data
    createDefaultDatabaseTable();

    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Nobi Nobita");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 10);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *(pContentValues.get()));
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // call query with distinct false, and verify query result row count is 3
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");

    SqliteCursor::Ptr pCursor1 = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false);

    RowElement expectedRows1[] = {
        {1, "Bart Simpson", "", 12},
        {2, "Lisa Simpson", "", 10},
        {3, "Nobi Nobita", "", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedRows1); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor1, expectedRows1[i]));
    }

    EXPECT_EQ(3, pCursor1->getCount());

    // When: call query with distinct true
    SqliteCursor::Ptr pCursor2 = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, true);

    // Then: verify query result, duplicate value should be removed
    RowElement expectedRows2[] = {
        {1, "Bart Simpson", "", 12},
        {2, "Lisa Simpson", "", 10},
        {3, "Nobi Nobita", "", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedRows2); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor2, expectedRows2[i]));
    }

    EXPECT_EQ(3, pCursor2->getCount());
}

TEST_F(SqliteDatabaseIntegrationTest, query_Succeeds_ByAnotherInstance)
{
    // Given: create database
    createDefaultDatabaseTable();

    // query and verify result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    // When: OpenDatabase by using another helper, and query database
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper::Ptr pHelper = new PartialMockSqliteOpenHelper(databasePath, 1);
    EXPECT_CALL(*pHelper, onCreate(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pHelper, onConfigure(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pHelper, onOpen(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pHelper, onUpgrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pHelper, onDowngrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());

    //
    SqliteDatabase::Ptr pDb = pHelper->getWritableDatabase();

    SqliteCursor::Ptr pCursor1 = pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false);


    // Then: verify query result
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor1, expectedExistRows[i]));
    }

    pHelper->close();
}

TEST_F(SqliteDatabaseIntegrationTest, query_ReturnsQueryResult_WhenDatabaseHasNoData)
{
    // Given: create database
    createDefaultDatabaseTableWithoutData();

    // When: query database
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    // Then: the cursor does not have raw
    EXPECT_EQ(0, pCursor->getCount());
}

TEST_F(SqliteDatabaseIntegrationTest, query_ThrowsSqlIllegalStateException_AfterClose)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: close SqliteDatabase
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->close();

    // Then: query throws exception
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");

    EASYHTTPCPP_ASSERT_THROW(pDb->query(DatabaseTableName, &columns, NULL, NULL, NULL, NULL, NULL, NULL, false),
            SqlIllegalStateException, 100201);
}

TEST_F(SqliteDatabaseIntegrationTest, execSql_Succeeds_WhenExecuteInsert)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmNone);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->execSql(insertString);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
        {3, "Homer Simpson", "Springfield", 38}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, execSql_Succeeds_WhenExecuteUpdate)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    std::string whereClause = "Age = 10";
    std::string updateString = SqliteQueryBuilder::buildUpdateString(DatabaseTableName, *pContentValues,
            &whereClause, SqliteConflictAlgorithmNone);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->execSql(updateString);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 38},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, execSql_Succeeds_WhenExecuteDelete)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    std::string whereClause = "Age = 10";
    std::string deleteString = SqliteQueryBuilder::buildDeleteString(DatabaseTableName, &whereClause);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->execSql(deleteString);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest,
        insertWithConflictAlgorithmNone_ThrowsSqlExecutionException_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmNone);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->execSql(insertString), SqlExecutionException, 100202);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest,
        insertWithConflictAlgorithmRollback_ThrowsSqlExecutionException_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmRollback);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->execSql(insertString), SqlExecutionException, 100202);

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest,
        insertWithConflictAlgorithmAbort_ThrowsSqlExecutionException_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmAbort);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->execSql(insertString), SqlExecutionException, 100202);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest,
        insertWithConflictAlgorithmFail_ThrowsSqlExecutionException_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmFail);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    EASYHTTPCPP_ASSERT_THROW_WITH_CAUSE(pDb->execSql(insertString), SqlExecutionException, 100202);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest,
        insertWithConflictAlgorithmIgnore_Succeeds_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmIgnore);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->execSql(insertString);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Tokyo", 15},
        {3, "Lisa Simpson", "Tokyo", 15}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, insertWithConflictAlgorithmReplace_Succeeds_WhenTryToInsertDataTheNameIsRedundant)
{
    // Given: create database
    createDefaultDatabaseTable();

    // When: insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Lisa Simpson");
    pContentValues->put("Address", "Tokyo");
    pContentValues->put("Age", 15);

    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmReplace);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    AutoSqliteTransaction autoTransaction(pDb);
    pDb->execSql(insertString);
    pDb->setTransactionSuccessful();

    // Then: query Age, and Name, and verify query result
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    columns.push_back("Id");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    EXPECT_EQ(2, pCursor->getCount());

    EXPECT_TRUE(databaseHasColumns(pCursor, 4, columns));

    RowElement expectedExistRows[] = {
        {1, "Bart Simpson", "Springfield", 12},
        {3, "Lisa Simpson", "Tokyo", 15},
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedExistRows); i++) {
        EXPECT_TRUE(databaseHasRow(pCursor, expectedExistRows[i]));
    }

    RowElement expectedNonExistRows[] = {
        {2, "Lisa Simpson", "Springfield", 10}
    };
    for (size_t i = 0; i < ELEMENT_NUM(expectedNonExistRows); i++) {
        EXPECT_FALSE(databaseHasRow(pCursor, expectedNonExistRows[i]));
    }
}

TEST_F(SqliteDatabaseIntegrationTest, setVersion_Succeeds)
{
    // Given: create database
    Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
    Poco::Data::SQLite::Connector::registerConnector();
    SqliteDatabase::Ptr pDb = SqliteDatabase::openOrCreateDatabase(databaseFilePath.toString());

    // When: set version
    EXPECT_EQ(0, pDb->getVersion());
    pDb->setVersion(1);

    // Then: version has been updated
    EXPECT_EQ(1, pDb->getVersion());
}

TEST_F(SqliteDatabaseIntegrationTest, setVersion_ThrowsSqlIllegalStateException_AfterClose)
{
    // Given: create database
    Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
    Poco::Data::SQLite::Connector::registerConnector();
    SqliteDatabase::Ptr pDb = SqliteDatabase::openOrCreateDatabase(databaseFilePath.toString());

    // When: Close the database
    pDb->close();

    // Then: throws SqlIllegalStateException
    EASYHTTPCPP_ASSERT_THROW(pDb->setVersion(1), SqlIllegalStateException, 100201);
}

TEST_F(SqliteDatabaseIntegrationTest, setAutoVacuum_Succeeds)
{
    // Given: create database
    Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
    Poco::Data::SQLite::Connector::registerConnector();
    SqliteDatabase::Ptr pDb = SqliteDatabase::openOrCreateDatabase(databaseFilePath.toString());

    // When: set AutoVacuumIncremental
    EXPECT_EQ(SqliteDatabase::AutoVacuumNone, pDb->getAutoVacuum());
    pDb->setAutoVacuum(SqliteDatabase::AutoVacuumIncremental);

    // Then: AutoVacuum has been updated
    EXPECT_EQ(SqliteDatabase::AutoVacuumIncremental, pDb->getAutoVacuum());
}

TEST_F(SqliteDatabaseIntegrationTest, setAutoVacuum_ThrowsSqlIllegalStateException_AfterClose)
{
    // Given: create database
    Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
    Poco::Data::SQLite::Connector::registerConnector();
    SqliteDatabase::Ptr pDb = SqliteDatabase::openOrCreateDatabase(databaseFilePath.toString());

    // When: Close the database
    pDb->close();

    // Then: throws SqlIllegalStateException
    EASYHTTPCPP_ASSERT_THROW(pDb->setAutoVacuum(SqliteDatabase::AutoVacuumIncremental), SqlIllegalStateException, 100201);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
