/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"

#include "SqliteDatabaseIntegrationTestConstants.h"
#include "SqliteDatabaseTestUtil.h"


using easyhttpcpp::testutil::PartialMockSqliteOpenHelper;

namespace easyhttpcpp {
namespace db {
namespace test {

namespace {
const std::string DatabaseDirString = SqliteDatabaseIntegrationTestConstants::DatabaseDir;
const std::string DatabaseFileName = SqliteDatabaseIntegrationTestConstants::DatabaseFileName;
const std::string DatabaseTableName = SqliteDatabaseIntegrationTestConstants::DatabaseTableName;
} /* namespace */

class DatabaseOperationTask : public Poco::Runnable {
public:

    DatabaseOperationTask()
    {
    }

    virtual ~DatabaseOperationTask()
    {
    }

    void setParam(const std::string& name, const std::string& address, int age)
    {
        m_name = name;
        m_address = address;
        m_age = age;
    }

    void run()
    {
        // Open Database and insert data and close
        Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
        PartialMockSqliteOpenHelper::Ptr pHelper = new PartialMockSqliteOpenHelper(databasePath, 1);
        EXPECT_CALL(*pHelper, onCreate(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(*pHelper, onConfigure(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(*pHelper, onOpen(testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(*pHelper, onUpgrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
        EXPECT_CALL(*pHelper, onDowngrade(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());

        SqliteDatabase::Ptr pDb = pHelper->getWritableDatabase();

        Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
        pContentValues->put("Name", m_name);
        pContentValues->put("Address", m_address);
        pContentValues->put("Age", m_age);

        pDb->beginTransaction();
        std::string sql = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
                SqliteConflictAlgorithmNone);
        pDb->execSql(sql);

        pDb->setTransactionSuccessful();
        pDb->endTransaction();

        pDb->close();
    }

private:
    std::string m_name;
    std::string m_address;
    int m_age;
};

class SqliteIntegrationTest : public testing::Test {
protected:

    void TearDown()
    {
        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
        m_pTestUtil->clearDatabaseFiles(databaseFilePath);
    }

    void createDefaultDatabaseTable()
    {
        // create table and insert data
        // table is as below
        // | Name           | Address       | Age |
        // | "Bart Simpson" | "Springfield" | 12  |
        // | "Lisa Simpson" | "Springfield" | 10  |
        //

        std::string columnString = "Name VARCHAR(30), Address VARCHAR, Age INTEGER(3), PRIMARY KEY(Name)";

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

    SqliteCursor::Ptr queryDatabase(const std::string& tableName, const std::vector<std::string>* columns)
    {
        return m_pTestUtil->queryDatabase(tableName, columns, NULL, NULL);
    }

    struct queryResult {
        char m_name[20];
        char m_address[20];
        int m_age;
    };

    SqliteDatabaseTestUtil::Ptr m_pTestUtil;
};

TEST_F(SqliteIntegrationTest, databaseOperationScenario)
{
    // create and open database
    //prepareDatabase(DB_FILE_PATH, DB_FILE_NAME, 1);
    createDefaultDatabaseTable();

    // query Age, and Name
    //queryDatabase();
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    // Then: verify query result
    queryResult result[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Lisa Simpson", "Springfield", 10}
    };
    EXPECT_EQ(3, pCursor->getColumnCount());
    EXPECT_EQ(0, pCursor->getColumnIndex("Age"));
    EXPECT_EQ(1, pCursor->getColumnIndex("Name"));
    EXPECT_EQ(2, pCursor->getColumnIndex("Address"));

    EXPECT_TRUE(pCursor->moveToFirst());
    int rowCount = 0;
    do {
        EXPECT_EQ(result[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(result[rowCount].m_name, pCursor->getString(1).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(2, rowCount);

    ////////////////////////////////////////////////////////////
    // insert data
    Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
    pContentValues->put("Name", "Homer Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 38);

    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->beginTransaction();
    pDb->insert(DatabaseTableName, *pContentValues);
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // query and verify query result
    pCursor = queryDatabase(DatabaseTableName, &columns);

    queryResult insertResult[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Lisa Simpson", "Springfield", 10},
        {"Homer Simpson", "Springfield", 38}
    };

    EXPECT_TRUE(pCursor->moveToFirst());
    rowCount = 0;
    do {
        EXPECT_EQ(insertResult[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(insertResult[rowCount].m_name, pCursor->getString(1).c_str());
        EXPECT_STREQ(insertResult[rowCount].m_address, pCursor->getString(2).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(3, rowCount);

    ////////////////////////////////////////////////////////////
    // delete data
    std::string whereClause = "Age = 10";
    std::string deleteString = SqliteQueryBuilder::buildDeleteString(DatabaseTableName, &whereClause);
    pDb->beginTransaction();
    pDb->execSql(deleteString);
    pDb->setTransactionSuccessful();
    pDb->endTransaction();

    // query and verify query result
    pCursor = queryDatabase(DatabaseTableName, &columns);

    queryResult deleteResult[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Homer Simpson", "Springfield", 38}
    };

    EXPECT_TRUE(pCursor->moveToFirst());
    rowCount = 0;
    do {
        EXPECT_EQ(deleteResult[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(deleteResult[rowCount].m_name, pCursor->getString(1).c_str());
        EXPECT_STREQ(deleteResult[rowCount].m_address, pCursor->getString(2).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(2, rowCount);

    ////////////////////////////////////////////////////////////
    // insert
    pContentValues->put("Name", "Marge Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 34);

    pDb->beginTransaction();
    std::string insertString = SqliteQueryBuilder::buildInsertString(DatabaseTableName, *pContentValues,
            SqliteConflictAlgorithmNone);
    pDb->execSql(insertString);
    pDb->setTransactionSuccessful();
    pDb->endTransaction();


    // query and verify query result
    pCursor = queryDatabase(DatabaseTableName, &columns);

    queryResult insertResult2[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Homer Simpson", "Springfield", 38},
        {"Marge Simpson", "Springfield", 34}
    };

    EXPECT_TRUE(pCursor->moveToFirst());
    rowCount = 0;
    do {
        EXPECT_EQ(insertResult2[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(insertResult2[rowCount].m_name, pCursor->getString(1).c_str());
        EXPECT_STREQ(insertResult2[rowCount].m_address, pCursor->getString(2).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(3, rowCount);

    ////////////////////////////////////////////////////////////
    // update data. replace it if conflict
    pContentValues->put("Name", "Maggie Simpson");
    pContentValues->put("Address", "Springfield");
    pContentValues->put("Age", 1);

    pDb->beginTransaction();
    whereClause = "Age = 34";
    std::string updateString = SqliteQueryBuilder::buildUpdateString(DatabaseTableName, *pContentValues,
            &whereClause, SqliteConflictAlgorithmReplace);

    pDb->execSql(updateString);

    pDb->setTransactionSuccessful();
    pDb->endTransaction();


    // query and verify query result
    pCursor = queryDatabase(DatabaseTableName, &columns);

    queryResult updateResult[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Homer Simpson", "Springfield", 38},
        {"Maggie Simpson", "Springfield", 1}
    };

    EXPECT_TRUE(pCursor->moveToFirst());
    rowCount = 0;
    do {
        EXPECT_EQ(updateResult[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(updateResult[rowCount].m_name, pCursor->getString(1).c_str());
        EXPECT_STREQ(updateResult[rowCount].m_address, pCursor->getString(2).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(3, rowCount);
}

TEST_F(SqliteIntegrationTest, query_Succeeds_WhenAnotherThreadCloseDatabase)
{
    // Given: create database
    createDefaultDatabaseTable();

    // access to the database by another thread, and close it after access.
    Poco::Thread thread;
    DatabaseOperationTask task;
    task.setParam("Nobi Nobita", "Tokyo", 10);
    thread.start(task);
    thread.join();

    // When: query database when another thread close database
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    // Then: verify query result
    queryResult result[10] = {
        {"Bart Simpson", "", 12},
        {"Lisa Simpson", "", 10},
        {"Nobi Nobita", "", 10}
    };
    EXPECT_EQ(2, pCursor->getColumnCount());
    EXPECT_EQ(0, pCursor->getColumnIndex("Age"));
    EXPECT_EQ(1, pCursor->getColumnIndex("Name"));

    EXPECT_TRUE(pCursor->moveToFirst());
    int rowCount = 0;
    do {
        EXPECT_EQ(result[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(result[rowCount].m_name, pCursor->getString(1).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(3, rowCount);
}

TEST_F(SqliteIntegrationTest, sqliteCursorOperation_Succeeds_AfterCloseSqliteDatabase)
{
    // Given: create and open database
    //prepareDatabase(DB_FILE_PATH, DB_FILE_NAME, 1);
    createDefaultDatabaseTable();

    // query Age, and Name
    //queryDatabase();
    std::vector<std::string> columns;
    columns.push_back("Age");
    columns.push_back("Name");
    columns.push_back("Address");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns);

    // When: close SsqliteDatabase
    SqliteDatabase::Ptr pDb = m_pTestUtil->getDatabase();
    pDb->close();

    // Then: verify query result
    queryResult result[10] = {
        {"Bart Simpson", "Springfield", 12},
        {"Lisa Simpson", "Springfield", 10}
    };
    EXPECT_EQ(3, pCursor->getColumnCount());
    EXPECT_EQ(0, pCursor->getColumnIndex("Age"));
    EXPECT_EQ(1, pCursor->getColumnIndex("Name"));
    EXPECT_EQ(2, pCursor->getColumnIndex("Address"));

    EXPECT_TRUE(pCursor->moveToFirst());
    int rowCount = 0;
    do {
        EXPECT_EQ(result[rowCount].m_age, pCursor->getInt(0));
        EXPECT_STREQ(result[rowCount].m_name, pCursor->getString(1).c_str());
        rowCount++;
    } while (pCursor->moveToNext());

    EXPECT_EQ(2, rowCount);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
