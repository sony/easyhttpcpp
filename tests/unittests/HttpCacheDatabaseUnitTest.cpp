/*
 * Copyright 2019 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "easyhttpcpp/db/SqlException.h"
#include "EasyHttpCppAssertions.h"

#include "HttpCacheDatabase.h"

using easyhttpcpp::db::SqlDatabaseCorruptException;
using easyhttpcpp::db::SqliteDatabase;

namespace easyhttpcpp {
namespace test {

namespace {

static const char* const Key1 = "key1";
static const char* const DatabaseFilePath = "database.db";

class PartialMockHttpCacheDatabaseOpenHelper : public HttpCacheDatabaseOpenHelper {
public:
    typedef Poco::AutoPtr<PartialMockHttpCacheDatabaseOpenHelper> Ptr;

    PartialMockHttpCacheDatabaseOpenHelper(const Poco::Path& databaseFile) : HttpCacheDatabaseOpenHelper(databaseFile)
    {
        ON_CALL(*this, getWritableDatabase())
            .WillByDefault(testing::Invoke(this, &PartialMockHttpCacheDatabaseOpenHelper::invokeGetWritableDatabase));
        ON_CALL(*this, getReadableDatabase())
            .WillByDefault(testing::Invoke(this, &PartialMockHttpCacheDatabaseOpenHelper::invokeGetReadableDatabase));
    }

    SqliteDatabase::Ptr invokeGetWritableDatabase()
    {
        return HttpCacheDatabaseOpenHelper::getWritableDatabase();
    }

    SqliteDatabase::Ptr invokeGetReadableDatabase()
    {
        return HttpCacheDatabaseOpenHelper::getReadableDatabase();
    }

    MOCK_METHOD0(getWritableDatabase, SqliteDatabase::Ptr());
    MOCK_METHOD0(getReadableDatabase, SqliteDatabase::Ptr());
};

}

TEST(HttpCacheDatabaseUnitTest,
        getMetadata_ThrowsSqlDatabaseCorruptException_WhenGetReadableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getReadableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getReadableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: getMetadata
    // Then: throws SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pTestee->getMetadata(Key1), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        deleteMetadata_ThrowsSqlDatabaseCorruptException_WhenGetWritableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getWritableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getWritableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: deleteMetadata
    // Then: throws SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pTestee->deleteMetadata(Key1), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        updateMetadata_ThrowsSqlDatabaseCorruptException_WhenGetWritableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getWritableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getWritableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: updateMetadata
    // Then: throws SqlDatabaseCorruptException
    HttpCacheMetadata::Ptr pHttpCacheMetadata;
    EASYHTTPCPP_EXPECT_THROW(pTestee->updateMetadata(Key1, pHttpCacheMetadata), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        updateLastAccessedSec_ThrowsSqlDatabaseCorruptException_WhenGetWritableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getWritableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getWritableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: updateLastAccessedSec
    // Then: throws SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pTestee->updateLastAccessedSec(Key1), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        enumerate_ThrowsSqlDatabaseCorruptException_WhenGetReadableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getReadableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getReadableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: enumerate
    // Then: throws SqlDatabaseCorruptException
    HttpCacheEnumerationListener* pListener = NULL;
    EASYHTTPCPP_EXPECT_THROW(pTestee->enumerate(pListener), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        getMetadataAll_ThrowsSqlDatabaseCorruptException_WhenGetReadableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getReadableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getReadableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: getMetadataAll
    // Then: throws SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pTestee->getMetadataAll(Key1), SqlDatabaseCorruptException, 100203);
}

TEST(HttpCacheDatabaseUnitTest,
        updateMetadataAll_ThrowsSqlDatabaseCorruptException_WhenGetWritableDatabaseThrewSqlDatabaseCorruptException)
{
    // Given: getWritableDatabase throws SqlDatabaseCorruptException
    PartialMockHttpCacheDatabaseOpenHelper::Ptr pMockOpenHelper =
            new testing::NiceMock<PartialMockHttpCacheDatabaseOpenHelper>(DatabaseFilePath);
    HttpCacheDatabase::Ptr pTestee = new HttpCacheDatabase(pMockOpenHelper);
    EXPECT_CALL(*pMockOpenHelper, getWritableDatabase())
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: updateMetadata
    // Then: throws SqlDatabaseCorruptException
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pHttpCacheMetadata;
    EASYHTTPCPP_EXPECT_THROW(pTestee->updateMetadataAll(Key1, pHttpCacheMetadata), SqlDatabaseCorruptException, 100203);
}

} /* namespace test */
} /* namespace easyhttpcpp */
