/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/db/SqlException.h"
#include "EasyHttpCppAssertions.h"
#include "PartialMockSqliteOpenHelper.h"
#include "TestLogger.h"
#include "SqliteDatabaseIntegrationTestConstants.h"

using namespace ::testing;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::PartialMockSqliteOpenHelper;

namespace easyhttpcpp {
namespace db {
namespace test {

namespace {
const std::string DatabaseDirString = SqliteDatabaseIntegrationTestConstants::DatabaseDir;
const std::string DatabaseFileName = SqliteDatabaseIntegrationTestConstants::DatabaseFileName;
} /* namespace */

class SqliteOpenHelperIntegrationTest : public testing::Test {
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

        Poco::Path databaseFilePath(DatabaseDirString, DatabaseFileName);
        Poco::File databaseFile(databaseFilePath.absolute().toString());
        FileUtil::removeFileIfPresent(databaseFile);
    }
};

TEST_F(SqliteOpenHelperIntegrationTest,
        constructor_ThrowsIllegalArgumentException_WhenTheDatabaseVersionTryingToCreateIsLessThan1)
{
    // Given: -

    // When: create SqliteOpenHelper instance the version is 0
    // Then: constructor throws SqlIllegalArgumentException
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    EASYHTTPCPP_ASSERT_THROW(PartialMockSqliteOpenHelper(databasePath, 0), SqlIllegalArgumentException, 100200);
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_Succeeds_WhenInitialCreationOfDatabase)
{
    //Given: create SqliteOpenHelper instance
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getReadableDatabase
    SqliteDatabase::Ptr pDb = sqliteOpenHelper.getReadableDatabase();

    // Then: getReadableDatabase succeeds
    EXPECT_FALSE(pDb.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_Succeeds_WhenCallItTwice)
{
    //Given: create SqliteOpenHelper instance
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getReadableDatabase twice
    SqliteDatabase::Ptr pDb1 = sqliteOpenHelper.getReadableDatabase();
    SqliteDatabase::Ptr pDb2 = sqliteOpenHelper.getReadableDatabase();

    // Then: getReadableDatabase succeeds
    EXPECT_FALSE(pDb1.isNull());
    EXPECT_FALSE(pDb2.isNull());
    EXPECT_TRUE(pDb1.get() == pDb2.get());
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_Succeeds_WhenUpgradeTheVersion)
{
    // Given: : create SqliteOpenHelper instance(version 1)
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelperVer1(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelperVer1, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onOpen(_)).Times(1);

    SqliteDatabase::Ptr pDb1 = sqliteOpenHelperVer1.getReadableDatabase();
    EXPECT_FALSE(pDb1.isNull());

    // When: create SqliteOpenHelper instance(version 2)
    PartialMockSqliteOpenHelper sqliteOpenHelperVer2(databasePath, 2);

    EXPECT_CALL(sqliteOpenHelperVer2, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onUpgrade(_, _, _)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onOpen(_)).Times(1);

    // When: call getReadableDatabase
    SqliteDatabase::Ptr pDb2 = sqliteOpenHelperVer2.getReadableDatabase();

    // Then: getReadableDatabase succeeds
    EXPECT_FALSE(pDb2.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_Succeeds_WhenDowngradeTheVersion)
{
    // Given: : create SqliteOpenHelper instance(version 2)
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelperVer2(databasePath, 2);

    EXPECT_CALL(sqliteOpenHelperVer2, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onOpen(_)).Times(1);

    SqliteDatabase::Ptr pDb2 = sqliteOpenHelperVer2.getReadableDatabase();
    EXPECT_FALSE(pDb2.isNull());

    // When: create SqliteOpenHelper instance(version 1)
    PartialMockSqliteOpenHelper sqliteOpenHelperVer1(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelperVer1, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onDowngrade(_, _, _)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onOpen(_)).Times(1);

    // When: call getReadableDatabase
    SqliteDatabase::Ptr pDb1 = sqliteOpenHelperVer1.getReadableDatabase();

    // Then: getReadableDatabase succeeds
    EXPECT_FALSE(pDb1.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_Succeeds_WhenAfterCloseSqliteOpenHelper)
{
    //Given: create SqliteOpenHelper instance and close the helper
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);
    sqliteOpenHelper.close();

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getReadableDatabase
    SqliteDatabase::Ptr pDb = sqliteOpenHelper.getReadableDatabase();

    // Then: getReadableDatabase succeeds
    EXPECT_FALSE(pDb.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getReadableDatabase_ThrowsIllegalStateException_WhenDatabasePathIsEmpty)
{
    //Given: create SqliteOpenHelper instance and close the helper
    Poco::Path databasePath;
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(0);

    // When: call getReadableDatabase
    // Then: getReadableDatabase throws 
    EASYHTTPCPP_ASSERT_THROW(sqliteOpenHelper.getReadableDatabase(), SqlIllegalStateException, 100201);
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_Succeeds_WhenInitialCreationOfDatabase)
{
    //Given: create SqliteOpenHelper instance
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getWritableDatabase
    SqliteDatabase::Ptr pDb = sqliteOpenHelper.getWritableDatabase();

    // Then: getWritableDatabase succeeds
    EXPECT_FALSE(pDb.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_Succeeds_WhenCallItTwice)
{
    //Given: create SqliteOpenHelper instance
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getWritableDatabase twice
    SqliteDatabase::Ptr pDb1 = sqliteOpenHelper.getWritableDatabase();
    SqliteDatabase::Ptr pDb2 = sqliteOpenHelper.getWritableDatabase();

    // Then: getWritableDatabase succeeds
    EXPECT_FALSE(pDb1.isNull());
    EXPECT_FALSE(pDb2.isNull());
    EXPECT_TRUE(pDb1.get() == pDb2.get());
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_Succeeds_WhenUpgradeTheVersion)
{
    // Given: : create SqliteOpenHelper instance(version 1)
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelperVer1(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelperVer1, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onOpen(_)).Times(1);

    SqliteDatabase::Ptr pDb1 = sqliteOpenHelperVer1.getWritableDatabase();
    EXPECT_FALSE(pDb1.isNull());

    // When: create SqliteOpenHelper instance(version 2)
    PartialMockSqliteOpenHelper sqliteOpenHelperVer2(databasePath, 2);

    EXPECT_CALL(sqliteOpenHelperVer2, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onUpgrade(_, _, _)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onOpen(_)).Times(1);

    // When: call getWritableDatabase
    SqliteDatabase::Ptr pDb2 = sqliteOpenHelperVer2.getWritableDatabase();

    // Then: getWritableDatabase succeeds
    EXPECT_FALSE(pDb2.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_Succeeds_WhenDowngradeTheVersion)
{
    // Given: : create SqliteOpenHelper instance(version 2)
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelperVer2(databasePath, 2);

    EXPECT_CALL(sqliteOpenHelperVer2, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer2, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer2, onOpen(_)).Times(1);

    SqliteDatabase::Ptr pDb2 = sqliteOpenHelperVer2.getWritableDatabase();
    EXPECT_FALSE(pDb2.isNull());

    // When: create SqliteOpenHelper instance(version 1)
    PartialMockSqliteOpenHelper sqliteOpenHelperVer1(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelperVer1, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelperVer1, onDowngrade(_, _, _)).Times(1);
    EXPECT_CALL(sqliteOpenHelperVer1, onOpen(_)).Times(1);

    // When: call getWritableDatabase
    SqliteDatabase::Ptr pDb1 = sqliteOpenHelperVer1.getWritableDatabase();

    // Then: getWritableDatabase succeeds
    EXPECT_FALSE(pDb1.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_Succeeds_WhenAfterCloseSqliteOpenHelper)
{
    //Given: create SqliteOpenHelper instance and close the helper
    Poco::Path databasePath(DatabaseDirString, DatabaseFileName);
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);
    sqliteOpenHelper.close();

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(1);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(1);

    // When: call getWritableDatabase
    SqliteDatabase::Ptr pDb = sqliteOpenHelper.getWritableDatabase();

    // Then: getWritableDatabase succeeds
    EXPECT_FALSE(pDb.isNull());
}

TEST_F(SqliteOpenHelperIntegrationTest, getWritableDatabase_ThrowsIllegalStateException_WhenDatabasePathIsEmpty)
{
    //Given: create SqliteOpenHelper instance and close the helper
    Poco::Path databasePath;
    PartialMockSqliteOpenHelper sqliteOpenHelper(databasePath, 1);

    EXPECT_CALL(sqliteOpenHelper, onConfigure(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onCreate(_)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onUpgrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onDowngrade(_, _, _)).Times(0);
    EXPECT_CALL(sqliteOpenHelper, onOpen(_)).Times(0);

    // When: call getWritableDatabase
    // Then: getWritableDatabase throws SqlIllegalStateException
    EASYHTTPCPP_ASSERT_THROW(sqliteOpenHelper.getWritableDatabase(), SqlIllegalStateException, 100201);
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
