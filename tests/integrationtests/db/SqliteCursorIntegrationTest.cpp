/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqliteOpenHelper.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"
#include "easyhttpcpp/db/SqlException.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

#include "SqliteDatabaseIntegrationTestConstants.h"
#include "SqliteDatabaseTestUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace db {
namespace test {

namespace {
const std::string DatabaseDirString = SqliteDatabaseIntegrationTestConstants::DatabaseDir;
const std::string DatabaseFileName = SqliteDatabaseIntegrationTestConstants::DatabaseFileName;
const std::string DatabaseTableName = SqliteDatabaseIntegrationTestConstants::DatabaseTableName;
const unsigned int DatabaseVersion = 1;
} /* namespace */

namespace {

struct SqliteCursorTestData {
    double m_double;
    float m_float;
    int m_int;
    long m_long;
    long long m_longlong;
    unsigned long long m_ulonglong;
    char m_char[20];
};

static const SqliteCursorTestData SqliteCursorTestDataSet[] = {
    {1.111111, 6.2222f, 1, -10, 825, 20160825, "string 1"},
    {2.111111, -7.2222f, -2, 11, -826, 20160826, "string 2"},
    {-3.111111, 8.2222f, 3, -12, 827, 20160827, "string 3"},
    {4.111111, 9.2222f, -4, 13, 828, 20160828, "string 4"},
    {5.111111, 10.2222f, 5, -14, -829, 20160829, "string 5"}
};

#define TEST_DATA_NUM (sizeof(SqliteCursorTestDataSet) / (sizeof SqliteCursorTestDataSet[0]))
}

class SqliteCursorIntegrationTest : public testing::Test {
protected:

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
        // | COL_D     | COL_F   | COL_I | COL_L | COL_LL | COL_ULL  | COL_S      |
        // |  1.111111 |  6.2222 |  1    | -10   |  825   | 20160825 | "string 1" |
        // |  2.111111 | -7.2222 | -2    |  11   | -826   | 20160826 | "string 2" |
        // | -3.111111 |  8.2222 |  3    | -12   |  827   | 20160827 | "string 3" |
        // |  4.111111 |  9.2222 | -4    |  13   |  828   | 20160828 | "string 4" |
        // |  5.111111 | 10.2222 |  5    | -14   | -829   | 20160829 | "string 5" |
        //
        std::string columnString =
                "COL_D REAL, COL_F REAL, COL_I INTEGER, COL_L INTEGER, COL_LL INTEGER, COL_ULL INTEGER, COL_S TEXT";

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
        m_pTestUtil = new SqliteDatabaseTestUtil();
        m_pTestUtil->createAndOpenDatabase(databaseFilePath, DatabaseVersion);
        m_pTestUtil->createTable(DatabaseTableName, columnString);

        for (size_t i = 0; i < TEST_DATA_NUM; i++) {
            Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
            pContentValues->put(m_pColumnNames[0], SqliteCursorTestDataSet[i].m_double);
            pContentValues->put(m_pColumnNames[1], SqliteCursorTestDataSet[i].m_float);
            pContentValues->put(m_pColumnNames[2], SqliteCursorTestDataSet[i].m_int);
            pContentValues->put(m_pColumnNames[3], SqliteCursorTestDataSet[i].m_long);
            pContentValues->put(m_pColumnNames[4], SqliteCursorTestDataSet[i].m_longlong);
            pContentValues->put(m_pColumnNames[5], SqliteCursorTestDataSet[i].m_ulonglong);
            pContentValues->put(m_pColumnNames[6], SqliteCursorTestDataSet[i].m_char);
            std::vector<ContentValues*> valuesVec;
            valuesVec.push_back(pContentValues);
            m_pTestUtil->insertData(DatabaseTableName, valuesVec);
        }
    }

    void createNullableDatabaseTable()
    {
        // create table and insert data
        // table is as below
        // | COL_D | COL_F   | COL_I | COL_L | COL_LL | COL_ULL  | COL_S |
        // |       | 6.2222  |  1    | -10   |  825   | 20160825 |       |
        //
        std::string columnString =
                "COL_D REAL, COL_F REAL, COL_I INTEGER, COL_L INTEGER, COL_LL INTEGER, COL_ULL INTEGER, COL_S TEXT";

        Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
        pContentValues->put(m_pColumnNames[1], SqliteCursorTestDataSet[0].m_float);
        pContentValues->put(m_pColumnNames[2], SqliteCursorTestDataSet[0].m_int);
        pContentValues->put(m_pColumnNames[3], SqliteCursorTestDataSet[0].m_long);
        pContentValues->put(m_pColumnNames[4], SqliteCursorTestDataSet[0].m_longlong);
        pContentValues->put(m_pColumnNames[5], SqliteCursorTestDataSet[0].m_ulonglong);

        //push back pContentValues and pContentValues2 to valuesVec
        std::vector<ContentValues*> valuesVec;
        valuesVec.push_back(pContentValues);

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);

        m_pTestUtil = new SqliteDatabaseTestUtil();
        m_pTestUtil->createAndOpenDatabase(databaseFilePath, DatabaseVersion);
        m_pTestUtil->createTable(DatabaseTableName, columnString);
        m_pTestUtil->insertData(DatabaseTableName, valuesVec);
    }

    SqliteCursor::Ptr queryDatabase(const std::string& tableName, const std::vector<std::string>* columns,
            const std::string* selection, const std::vector<std::string>* selectionArgs)
    {
        return m_pTestUtil->queryDatabase(tableName, columns, selection, selectionArgs);
    }

    void insertMultiData(const std::string& tableName, int count)
    {
        for (int i = 0; i < count; i++) {
            Poco::AutoPtr<ContentValues> pContentValues = new ContentValues();
            pContentValues->put(m_pColumnNames[0], SqliteCursorTestDataSet[0].m_double);
            pContentValues->put(m_pColumnNames[1], SqliteCursorTestDataSet[0].m_float);
            pContentValues->put(m_pColumnNames[2], i);
            pContentValues->put(m_pColumnNames[3], i);
            pContentValues->put(m_pColumnNames[4], i);
            pContentValues->put(m_pColumnNames[5], i);
            pContentValues->put(m_pColumnNames[6], StringUtil::format("string %d", i));
            std::vector<ContentValues*> valuesVec;
            valuesVec.push_back(pContentValues);
            m_pTestUtil->insertData(tableName, valuesVec);
        }
    }

    static const char* m_pColumnNames[];

    SqliteDatabaseTestUtil::Ptr m_pTestUtil;
};

const char* SqliteCursorIntegrationTest::m_pColumnNames[] = {
    "COL_D",
    "COL_F",
    "COL_I",
    "COL_L",
    "COL_LL",
    "COL_ULL",
    "COL_S",
};

TEST_F(SqliteCursorIntegrationTest, getCount_ReturnsProperRawCount_WhenQueryAllColumns)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnCount returns proper value
    EXPECT_EQ(5, pCursor->getCount());
}

TEST_F(SqliteCursorIntegrationTest, getCount_ReturnsProperRawCount_WhenQueryWithSelectionArgs)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 13
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("13");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: getColumnCount returns proper value
    EXPECT_EQ(1, pCursor->getCount());
}

TEST_F(SqliteCursorIntegrationTest, getCount_ReturnsProperRawCount_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: getColumnCount returns proper value
    EXPECT_EQ(0, pCursor->getCount());
}

TEST_F(SqliteCursorIntegrationTest, getCount_ReturnsProperRawCount_WhenLargeAmountOfQueryData)
{
    // Given: prepare database
    createDefaultDatabaseTable();
    insertMultiData(DatabaseTableName, 10000);

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnCount returns proper value
    // initial data(5) + inserted data(10000)
    EXPECT_EQ(10005, pCursor->getCount());
}

TEST_F(SqliteCursorIntegrationTest, getCount_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // call getCount
    EASYHTTPCPP_EXPECT_THROW(pCursor->getCount(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, getColumnCount_ReturnsProperColumnCount_WhenQueryAllColumns)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnCount returns proper value
    EXPECT_EQ(7, pCursor->getColumnCount());
}

TEST_F(SqliteCursorIntegrationTest, getColumnCount_ReturnsProperColumnCount_WhenQuerySpecifiedColumns)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    //SELECT COL_D, COL_I, COL_LL, COL_S table
    std::vector<std::string> columns;
    columns.push_back(m_pColumnNames[0]);
    columns.push_back(m_pColumnNames[2]);
    columns.push_back(m_pColumnNames[4]);
    columns.push_back(m_pColumnNames[6]);
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns, NULL, NULL);

    // Then: getColumnCount returns proper value
    EXPECT_EQ(4, pCursor->getColumnCount());
}

TEST_F(SqliteCursorIntegrationTest, getColumnCount_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getColumnCount Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getColumnCount(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, getPosition_ReturnsCurrentCursorPosition)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getPosition returns proper value
    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->moveToNext());
    EXPECT_EQ(1, pCursor->getPosition());
    EXPECT_TRUE(pCursor->moveToLast());
    EXPECT_EQ(4, pCursor->getPosition());
}

TEST_F(SqliteCursorIntegrationTest, getPosition_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // call getPosition
    EASYHTTPCPP_EXPECT_THROW(pCursor->getPosition(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, getColumnNames_ReturnsProperVectorString_WhenQueryAllColumns)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnNames returns proper value
    std::vector<std::string> names;
    names = pCursor->getColumnNames();
    for (size_t i = 0; i < names.size(); i++) {
        EXPECT_STREQ(m_pColumnNames[i], names.at(i).c_str());
    }
}

TEST_F(SqliteCursorIntegrationTest, getColumnNames_ReturnsProperVectorString_WhenQuerySpecifiedColumns)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    //SELECT COL_D, COL_I, COL_LL, COL_S table
    std::vector<std::string> columns;
    columns.push_back(m_pColumnNames[0]);
    columns.push_back(m_pColumnNames[2]);
    columns.push_back(m_pColumnNames[4]);
    columns.push_back(m_pColumnNames[6]);
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, &columns, NULL, NULL);

    // Then: getColumnNames returns proper value
    std::vector<std::string> names;
    names = pCursor->getColumnNames();
    EXPECT_EQ(4, names.size());
    EXPECT_STREQ(m_pColumnNames[0], names.at(0).c_str());
    EXPECT_STREQ(m_pColumnNames[2], names.at(1).c_str());
    EXPECT_STREQ(m_pColumnNames[4], names.at(2).c_str());
    EXPECT_STREQ(m_pColumnNames[6], names.at(3).c_str());
}

TEST_F(SqliteCursorIntegrationTest, getColumnNames_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getColumnNames Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getColumnNames(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, isFirstAndIsLast_ReturnProperValue_WhenRowCountIsLargerThanTwo)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: isFirst and isLast return proper value

    // initial state
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // After call moveToLast
    EXPECT_TRUE(pCursor->moveToLast());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());

    // After call moveToFast
    EXPECT_TRUE(pCursor->moveToFirst());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // After call moveToNext
    EXPECT_TRUE(pCursor->moveToNext());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, isFirstAndIsLast_ReturnProperValue_WhenRowCountIsOne)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM SqliteCursorTest WHERE COL_I = -2
    std::string selection = "COL_I = -2";
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, NULL);

    // Then: isFirst and isLast return proper value

    // initial state
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());

    // After call moveToLast
    EXPECT_TRUE(pCursor->moveToLast());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());

    // After call moveToFast
    EXPECT_TRUE(pCursor->moveToFirst());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());

    // After call moveToNext
    EXPECT_FALSE(pCursor->moveToNext());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, isFirst_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call isFirst Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->isFirst(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, isLast_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call isLast Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->isLast(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, getColumnIndex_ReturnsProperIndexValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnIndex returns proper index
    for (size_t i = 0; i < sizeof (m_pColumnNames) / sizeof (m_pColumnNames[0]); i++) {
        EXPECT_EQ(i, pCursor->getColumnIndex(m_pColumnNames[i]));
    }
}

TEST_F(SqliteCursorIntegrationTest, getColumnIndex_ThrowsSqlIllegalArgumentException_WhenCannotFindColumnName)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getColumnIndex throws SqlIllegalArgumentException
    //       when parameter column name is unknown
    EASYHTTPCPP_EXPECT_THROW(pCursor->getColumnIndex("COL_XXX"), SqlIllegalArgumentException, 100200);
}

TEST_F(SqliteCursorIntegrationTest, getColumnIndex_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getColumnIndex Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getColumnIndex(m_pColumnNames[0]), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, getDouble_ReturnsDoubleValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getDouble returns proper value
    size_t i = 0;
    do {
        EXPECT_DOUBLE_EQ(SqliteCursorTestDataSet[i].m_double, pCursor->getDouble(0));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getDouble_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getDouble throws exception for column type is not double
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getDouble(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getDouble_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getDouble throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getDouble(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getDouble_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getDouble Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getDouble(3), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetDouble_ReturnsDoubleValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetDouble returns proper value
    double defaultValue = 0.98765;
    size_t i = 0;
    do {
        EXPECT_DOUBLE_EQ(SqliteCursorTestDataSet[i].m_double, pCursor->optGetDouble(0, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetDouble_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetDouble returns default value
    double defaultValue = 0.98765;
    EXPECT_DOUBLE_EQ(defaultValue, pCursor->optGetDouble(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetDouble_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetDouble returns default value
    double defaultValue = 0.98765;
    EXPECT_DOUBLE_EQ(defaultValue, pCursor->optGetDouble(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetDouble_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetDouble returns default value
    double defaultValue = 0.98765;
    EXPECT_DOUBLE_EQ(defaultValue, pCursor->optGetDouble(0, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getFloat_ReturnsFloatValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getFloat returns proper value
    size_t i = 0;
    do {
        EXPECT_FLOAT_EQ(SqliteCursorTestDataSet[i].m_float, pCursor->getFloat(1));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getFloat_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getFloat throws exception for column type is not float
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getFloat(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getFloat_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getFloat throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getFloat(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getFloat_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getFloat Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getFloat(4), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetFloat_ReturnsFloatValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetFloat returns proper value
    float defaultValue = 0.12345f;
    size_t i = 0;
    do {
        EXPECT_FLOAT_EQ(SqliteCursorTestDataSet[i].m_float, pCursor->optGetFloat(1, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetFloat_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetFloat returns default value
    float defaultValue = 0.12345f;
    EXPECT_FLOAT_EQ(defaultValue, pCursor->optGetFloat(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetFloat_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetFloat returns default value
    float defaultValue = 0.12345f;
    EXPECT_FLOAT_EQ(defaultValue, pCursor->optGetFloat(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetFloat_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetFloat returns default value
    float defaultValue = 0.12345f;
    EXPECT_FLOAT_EQ(defaultValue, pCursor->optGetFloat(1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getInt_ReturnsIntValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getInt returns proper value
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_int, pCursor->getInt(2));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getInt_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getInt throws exception for column type is not integer
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getInt(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getInt_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getInt throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getInt(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getInt_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getInt Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getInt(0), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetInt_ReturnsIntValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetInt returns proper value
    int defaultValue = 1234;
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_int, pCursor->optGetInt(2, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetInt_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetInt returns default value
    int defaultValue = 1234;
    EXPECT_EQ(defaultValue, pCursor->optGetInt(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetInt_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetInt returns default value
    int defaultValue = 1234;
    EXPECT_EQ(defaultValue, pCursor->optGetInt(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetInt_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetInt returns default value
    int defaultValue = 1234;
    EXPECT_EQ(defaultValue, pCursor->optGetInt(2, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getLong_ReturnsLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database

    // SELECT * FROM SqliteCursorTest
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLong returns proper value
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_long, pCursor->getLong(3));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getLong_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLong throws exception for column type is not long
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getLong(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getLong_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLong throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getLong(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getLong_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getLong Fails
    // call getLong
    EASYHTTPCPP_EXPECT_THROW(pCursor->getLong(5), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetLong_ReturnsLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLong returns proper value
    long defaultValue = 123456;
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_long, pCursor->optGetLong(3, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetLong_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLong returns default value
    long defaultValue = 123456;
    EXPECT_EQ(defaultValue, pCursor->optGetLong(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetLong_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLong returns default value
    long defaultValue = 123456;
    EXPECT_EQ(defaultValue, pCursor->optGetLong(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetLong_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetLong returns default value
    long defaultValue = 123456;
    EXPECT_EQ(defaultValue, pCursor->optGetLong(3, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getLongLong_ReturnsLongLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLongLong returns proper value
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_longlong, pCursor->getLongLong(4));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getLongLong_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLongLong throws exception for column type is not long long
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getLongLong(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getLongLong_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getLongLong throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getLongLong(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getLongLong_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getLongLong Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getLongLong(4), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetLongLong_ReturnsLongLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLongLong returns proper value
    long long defaultValue = 654987;
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_longlong, pCursor->optGetLongLong(4, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetLongLong_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLongLong returns default value
    long long defaultValue = 654987;
    EXPECT_EQ(defaultValue, pCursor->optGetLongLong(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetLongLong_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetLongLong returns default value
    long long defaultValue = 654987;
    EXPECT_EQ(defaultValue, pCursor->optGetLongLong(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetLongLong_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetLongLong returns default value
    long long defaultValue = 654987;
    EXPECT_EQ(defaultValue, pCursor->optGetLongLong(4, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getUnsignedLongLong_ReturnsUnsignedLongLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getUnsignedLongLong returns proper value
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_ulonglong, pCursor->getUnsignedLongLong(5));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getUnsignedLongLong_ThrowsSqlExecutionException_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getUnsignedLongLong throws exception for column type is not unsigned long long
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getUnsignedLongLong(6), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getUnsignedLongLong_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getUnsignedLongLong throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getUnsignedLongLong(pCursor->getColumnCount() + 1), SqlExecutionException,
            100202);
}

TEST_F(SqliteCursorIntegrationTest, getUnsignedLongLong_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getLongLong Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getUnsignedLongLong(5), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetUnsignedLongLong_ReturnsUnsignedLongLongValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetUnsignedLongLong returns proper value
    unsigned long long defaultValue = 50018113806102;
    size_t i = 0;
    do {
        EXPECT_EQ(SqliteCursorTestDataSet[i].m_ulonglong, pCursor->optGetUnsignedLongLong(5, defaultValue));
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetUnsignedLongLong_ReturnsDefaultValue_WhenColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetUnsignedLongLong returns default value
    unsigned long long defaultValue = 50018113806102;
    EXPECT_EQ(defaultValue, pCursor->optGetUnsignedLongLong(6, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetUnsignedLongLong_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetUnsignedLongLong returns default value
    unsigned long long defaultValue = 50018113806102;
    EXPECT_EQ(defaultValue, pCursor->optGetUnsignedLongLong(pCursor->getColumnCount() + 1, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, optGetUnsignedLongLong_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetUnsignedLongLong returns default value
    unsigned long long defaultValue = 50018113806102;
    EXPECT_EQ(defaultValue, pCursor->optGetUnsignedLongLong(5, defaultValue));
}

TEST_F(SqliteCursorIntegrationTest, getString_ReturnsStringValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getString returns proper value
    size_t i = 0;
    do {
        EXPECT_STREQ(SqliteCursorTestDataSet[i].m_char, pCursor->getString(6).c_str());
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, getString_ReturnsValueAsString_EvenIfColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getString succeeds even if column type is integer
    EXPECT_STREQ("1.111111", pCursor->getString(0).c_str());
}

TEST_F(SqliteCursorIntegrationTest, getString_ThrowsSqlExecutionException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getString throws exception for column index is out of column count
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getString(pCursor->getColumnCount() + 1), SqlExecutionException, 100202);
}

TEST_F(SqliteCursorIntegrationTest, getString_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getString Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getString(1), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, optGetString_ReturnsStringValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetString returns proper value
    std::string defaultValue = "default string";
    size_t i = 0;
    do {
        EXPECT_STREQ(SqliteCursorTestDataSet[i].m_char, pCursor->optGetString(6, defaultValue).c_str());
        i++;
    } while (pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, optGetString_ReturnsValueAsString_EvenIfColumnTypeIsNotProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetString returns default value
    std::string defaultValue = "default string";
    EXPECT_STREQ("6.2222", pCursor->optGetString(1, defaultValue).c_str());
}

TEST_F(SqliteCursorIntegrationTest, optGetString_ReturnsDefaultValue_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: optGetString returns default value
    std::string defaultValue = "default string";
    EXPECT_STREQ(defaultValue.c_str(), pCursor->optGetString(pCursor->getColumnCount() + 1, defaultValue).c_str());
}

TEST_F(SqliteCursorIntegrationTest, optGetString_ReturnsDefaultValue_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);
    pCursor->close();

    // Then: optGetString returns default value
    std::string defaultValue = "default string";
    EXPECT_STREQ(defaultValue.c_str(), pCursor->optGetString(6, defaultValue).c_str());
}

TEST_F(SqliteCursorIntegrationTest, getType_Returns_ProperType)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getType returns proper value
    EXPECT_EQ(CursorFieldTypeString, pCursor->getType(0));
    EXPECT_EQ(CursorFieldTypeString, pCursor->getType(1));
    EXPECT_EQ(CursorFieldTypeInteger, pCursor->getType(2));
    EXPECT_EQ(CursorFieldTypeInteger, pCursor->getType(3));
    EXPECT_EQ(CursorFieldTypeInteger, pCursor->getType(4));
    EXPECT_EQ(CursorFieldTypeInteger, pCursor->getType(5));
    EXPECT_EQ(CursorFieldTypeString, pCursor->getType(6));
}

TEST_F(SqliteCursorIntegrationTest, getType_ThrowsSqlIllegalArgumentException_WhenIndexIsOutOfColumnCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: getType throws exception
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(pCursor->getType(pCursor->getColumnCount() + 1), SqlIllegalArgumentException, 100200);
}

TEST_F(SqliteCursorIntegrationTest, getType_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call getType Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->getType(0), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, isNull_ReturnsFalse_WhenColumnHasData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: isNull returns proper state
    EXPECT_FALSE(pCursor->isNull(0));
}

TEST_F(SqliteCursorIntegrationTest, isNull_ReturnsTrue_WhenColumnHasNoData)
{
    // Given: prepare database
    createNullableDatabaseTable();

    // When: query database
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: isNull returns false
    EXPECT_TRUE(pCursor->isNull(0));
}

TEST_F(SqliteCursorIntegrationTest, isNull_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call isNull Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->isNull(0), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, move_Succeeds_WhenOffestIsPositiveValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: move succeeds
    EXPECT_TRUE(pCursor->move(2));

    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, move_Succeeds_WhenOffestIsNegativeValue)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to last raw
    EXPECT_TRUE(pCursor->moveToLast());

    // Then: move succeeds
    EXPECT_TRUE(pCursor->move(-2));

    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, move_Fails_WhenOffestIsGreaterThanTheUpperLimitOfRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: move fails
    EXPECT_FALSE(pCursor->move(10));

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, move_Fails_WhenOffestIsLessThanTheLowerLimitOfRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: move fails
    EXPECT_FALSE(pCursor->move(-10));

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, move_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: move returns false
    EXPECT_FALSE(pCursor->move(1));
}

TEST_F(SqliteCursorIntegrationTest, move_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call move Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->move(1), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, moveToFirst_Succeeds_WhenCurrentCursorIsInFirstRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // Then: moveToFirst Succeeds
    EXPECT_TRUE(pCursor->moveToFirst());

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToFirst_Succeeds_WhenCurrentCursorIsNotInFirstRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to 3
    EXPECT_TRUE(pCursor->move(3));

    // Then: moveToFirst Succeeds
    EXPECT_TRUE(pCursor->moveToFirst());

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToFirst_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: moveToFirst returns false
    EXPECT_FALSE(pCursor->moveToFirst());
}

TEST_F(SqliteCursorIntegrationTest, moveToFirst_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call moveToFirst Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->moveToFirst(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, moveToLast_Succeeds_WhenCurrentCursorIsNotInLastRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to 1
    EXPECT_TRUE(pCursor->move(1));

    // Then: moveToLast Succeeds
    EXPECT_TRUE(pCursor->moveToLast());

    EXPECT_EQ(4, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToLast_Succeeds_WhenCurrentCursorIsInLastRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to last index
    EXPECT_TRUE(pCursor->moveToLast());

    // Then: moveToLast Succeeds
    EXPECT_TRUE(pCursor->moveToLast());

    EXPECT_EQ(4, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToLast_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: moveToLast returns false
    EXPECT_FALSE(pCursor->moveToLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToLast_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call moveToLast Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->moveToLast(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, moveToNext_Succeeds_WhenCurrentCursorIsNotInLastRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToNext Succeeds
    EXPECT_TRUE(pCursor->moveToNext());

    EXPECT_EQ(3, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToNext_Fails_WhenCurrentCursorIsInLastRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to last index
    EXPECT_TRUE(pCursor->moveToLast());
    EXPECT_EQ(4, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());

    // Then: moveToNext fails
    EXPECT_FALSE(pCursor->moveToNext());

    EXPECT_EQ(4, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_TRUE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToNext_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: moveToNext returns false
    EXPECT_TRUE(pCursor->moveToNext());
}

TEST_F(SqliteCursorIntegrationTest, moveToNext_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call moveToNext Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->moveToNext(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, moveToPrevious_Succeeds_WhenCurrentCursorIsNotInFisrtRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPrevious Succeeds
    EXPECT_TRUE(pCursor->moveToPrevious());

    EXPECT_EQ(1, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPrevious_Fails_WhenCurrentCursorIsInFisrtRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to first index
    EXPECT_TRUE(pCursor->moveToFirst());
    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPrevious Fails
    EXPECT_FALSE(pCursor->moveToPrevious());

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPrevious_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: moveToNext returns false
    EXPECT_TRUE(pCursor->moveToNext());
    EXPECT_TRUE(pCursor->moveToPrevious());
}

TEST_F(SqliteCursorIntegrationTest, moveToPrevious_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call moveToPrevious Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->moveToPrevious(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Succeeds_WhenCallItWithPositionValueZeroAndCurrentPositonIsNotInFirstRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 3
    EXPECT_TRUE(pCursor->move(3));
    EXPECT_EQ(3, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPosition succeeds
    EXPECT_TRUE(pCursor->moveToPosition(0));

    EXPECT_EQ(0, pCursor->getPosition());
    EXPECT_TRUE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Succeeds_WhenPositionValueIsLessThanTheCurrentRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPosition succeeds
    EXPECT_TRUE(pCursor->moveToPosition(1));

    EXPECT_EQ(1, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Succeeds_WhenPositionValueIsGreaterThanTheCurrentRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPosition succeeds
    EXPECT_TRUE(pCursor->moveToPosition(3));

    EXPECT_EQ(3, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Succeeds_WhenPositionValueIsEqaulToTheCurrentRow)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPosition succeeds
    EXPECT_TRUE(pCursor->moveToPosition(2));

    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Fails_WhenPositionValueIsGreaterThanTheTheRowCount)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // move cursor to index 2
    EXPECT_TRUE(pCursor->move(2));
    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());

    // Then: moveToPosition fails
    EXPECT_FALSE(pCursor->moveToPosition(10));

    EXPECT_EQ(2, pCursor->getPosition());
    EXPECT_FALSE(pCursor->isFirst());
    EXPECT_FALSE(pCursor->isLast());
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_Fails_WhenQueryReturnsNoData)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table where COL_L = 20
    std::string selection = "COL_L = ?";
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back("20");
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, &selection, &selectionArgs);

    // Then: moveToPosition returns false
    EXPECT_FALSE(pCursor->moveToPosition(1));
}

TEST_F(SqliteCursorIntegrationTest, moveToPosition_ThrowsSqlIllegalStateException_AfterCloseCursor)
{
    // Given: prepare database
    createDefaultDatabaseTable();

    // When: query database
    // SELECT * FROM table
    SqliteCursor::Ptr pCursor = queryDatabase(DatabaseTableName, NULL, NULL, NULL);

    // close SqliteCursor
    pCursor->close();

    // Then: call moveToPosition Fails
    EASYHTTPCPP_EXPECT_THROW(pCursor->moveToPosition(1), SqlIllegalStateException, 100201);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
