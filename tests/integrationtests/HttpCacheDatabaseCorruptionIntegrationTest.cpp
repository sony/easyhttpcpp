/*
 * Copyright 2019 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/CommonException.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/db/AutoSqliteDatabase.h"
#include "easyhttpcpp/db/SqlException.h"
#include "ForwardingMockSqliteDatabase.h"
#include "EasyHttpCppAssertions.h"
#include "TestLogger.h"

#include "HttpCacheDatabase.h"
#include "HttpCacheEnumerationListener.h"
#include "HttpFileCache.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::PocoException;
using easyhttpcpp::db::AutoSqliteDatabase;
using easyhttpcpp::db::SqlDatabaseCorruptException;
using easyhttpcpp::db::SqliteDatabase;
using easyhttpcpp::testutil::ForwardingMockSqliteDatabase;

namespace easyhttpcpp {
namespace test {

namespace {

const char* Test1Url = "http://localhost:9982/test1?a=10";
const char* const Test1ResponseBody = "test1 response body";
const char* const Test1TempFilename = "tempFile0001";

} /* namespace */

class HttpCacheDatabaseCorruptionIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    class TestHttpCacheEnumerationListener : public HttpCacheEnumerationListener {
    public:
        virtual bool onEnumerate(const EnumerationParam& param)
        {
            return true;
        }
    };
};

TEST_F(HttpCacheDatabaseCorruptionIntegrationTest,
        enumerate_ThrowsSqlDatabaseCorruptException_WhenThrowSqlDatabaseCorruptExceptionInQuery)
{
    // Given: prepare cache database and free HttpFileCache (SqliteDatabase)
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);
    HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
            strlen(Test1ResponseBody));
    std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
    ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));

    // HttpCacheDatabase
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabaseOpenHelper::Ptr pCacheDatabaseOpenHelper = new HttpCacheDatabaseOpenHelper(databasePath);
    HttpCacheDatabase::Ptr pCacheDatabase = new HttpCacheDatabase(pCacheDatabaseOpenHelper);

    // set up mock
    SqliteDatabase::Ptr pDb = pCacheDatabaseOpenHelper->getWritableDatabase();
    AutoSqliteDatabase autoSqliteDatabase(pDb);
    ForwardingMockSqliteDatabase::Ptr pMockDatabase =
            new testing::NiceMock<ForwardingMockSqliteDatabase>(databasePath.toString(), pDb);
    pCacheDatabaseOpenHelper->overrideInternalDatabase(pMockDatabase);

    // throw SqlDatabaseCorruptException from query of HttpCacheDatabase::enamerate
    EXPECT_CALL(*pMockDatabase, query(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
            testing::_, testing::_, testing::_))
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: enumerate
    // Then: throw SqlDatabaseCorruptException
    TestHttpCacheEnumerationListener enumerationListener;
    EASYHTTPCPP_EXPECT_THROW(pCacheDatabase->enumerate(&enumerationListener), SqlDatabaseCorruptException, 100203);
}

TEST_F(HttpCacheDatabaseCorruptionIntegrationTest,
        getMetadata_ThrowsSqlDatabaseCorruptException_WhenThrowSqlDatabaseCorruptExceptionInQuery)
{
    // Given: prepare cache database
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
            strlen(Test1ResponseBody));
    std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
    ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));

    // HttpCacheDatabase
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabaseOpenHelper::Ptr pCacheDatabaseOpenHelper = new HttpCacheDatabaseOpenHelper(databasePath);
    HttpCacheDatabase::Ptr pCacheDatabase = new HttpCacheDatabase(pCacheDatabaseOpenHelper);

    // set up mock
    SqliteDatabase::Ptr pDb = pCacheDatabaseOpenHelper->getWritableDatabase();
    AutoSqliteDatabase autoSqliteDatabase(pDb);
    ForwardingMockSqliteDatabase::Ptr pMockDatabase =
            new testing::NiceMock<ForwardingMockSqliteDatabase>(databasePath.toString(), pDb);
    pCacheDatabaseOpenHelper->overrideInternalDatabase(pMockDatabase);

    // throw SqlDatabaseCorruptException from query
    EXPECT_CALL(*pMockDatabase, query(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
            testing::_, testing::_, testing::_))
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: getMetadata
    // Then: throw SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pCacheDatabase->getMetadata(key), SqlDatabaseCorruptException, 100203);
}

TEST_F(HttpCacheDatabaseCorruptionIntegrationTest,
        updateLastAccessedSec_ThrowsSqlDatabaseCorruptException_WhenThrowSqlDatabaseCorruptExceptionInUpdate)
{
    // Given: prepare cache database
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
            strlen(Test1ResponseBody));
    std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
    ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));

    // HttpCacheDatabase
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabaseOpenHelper::Ptr pCacheDatabaseOpenHelper = new HttpCacheDatabaseOpenHelper(databasePath);
    HttpCacheDatabase::Ptr pCacheDatabase = new HttpCacheDatabase(pCacheDatabaseOpenHelper);

    // set up mock
    SqliteDatabase::Ptr pDb = pCacheDatabaseOpenHelper->getWritableDatabase();
    AutoSqliteDatabase autoSqliteDatabase(pDb);
    ForwardingMockSqliteDatabase::Ptr pMockDatabase =
            new testing::NiceMock<ForwardingMockSqliteDatabase>(databasePath.toString(), pDb);
    pCacheDatabaseOpenHelper->overrideInternalDatabase(pMockDatabase);

    // throw SqlDatabaseCorruptException from update of HttpCacheDatabase::updateLastAccessedSec
    EXPECT_CALL(*pMockDatabase, update(testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: updateLastAccessedSec
    // Then: throw SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pCacheDatabase->updateLastAccessedSec(key), SqlDatabaseCorruptException, 100203);
}

TEST_F(HttpCacheDatabaseCorruptionIntegrationTest,
        updateMetadata_ThrowsSqlDatabaseCorruptException_WhenThrowSqlDatabaseCorruptExceptionInReplace)
{
    // Given: prepare cache database
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
            strlen(Test1ResponseBody));
    std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
    ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));

    // HttpCacheDatabase
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabaseOpenHelper::Ptr pCacheDatabaseOpenHelper = new HttpCacheDatabaseOpenHelper(databasePath);
    HttpCacheDatabase::Ptr pCacheDatabase = new HttpCacheDatabase(pCacheDatabaseOpenHelper);

    // set up mock
    SqliteDatabase::Ptr pDb = pCacheDatabaseOpenHelper->getWritableDatabase();
    AutoSqliteDatabase autoSqliteDatabase(pDb);
    ForwardingMockSqliteDatabase::Ptr pMockDatabase =
            new testing::NiceMock<ForwardingMockSqliteDatabase>(databasePath.toString(), pDb);
    pCacheDatabaseOpenHelper->overrideInternalDatabase(pMockDatabase);

    // throw SqlDatabaseCorruptException from replace
    EXPECT_CALL(*pMockDatabase, replace(testing::_, testing::_))
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: updateMetadata
    // Then: throw SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pCacheDatabase->updateMetadata(key, pHttpCacheMetadata), SqlDatabaseCorruptException, 100203);
}

TEST_F(HttpCacheDatabaseCorruptionIntegrationTest,
        deleteMetadata_ThrowsSqlDatabaseCorruptException_WhenThrowSqlDatabaseCorruptExceptionInDeleteRows)
{
    // Given: prepare cache database
    std::string url = Test1Url;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);

    Poco::Path cacheRootDir(HttpTestUtil::getDefaultCacheRootDir());
    HttpFileCache::Ptr pHttpFileCache = new HttpFileCache(cacheRootDir, HttpTestConstants::DefaultCacheMaxSize);

    HttpCacheMetadata::Ptr pHttpCacheMetadata = HttpTestUtil::createHttpCacheMetadata(key, url,
            strlen(Test1ResponseBody));
    std::string tempFilePath = HttpTestUtil::createResponseTempFile(Test1TempFilename, Test1ResponseBody);
    ASSERT_TRUE(pHttpFileCache->put(key, pHttpCacheMetadata, tempFilePath));

    // HttpCacheDatabase
    Poco::Path databasePath(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabaseOpenHelper::Ptr pCacheDatabaseOpenHelper = new HttpCacheDatabaseOpenHelper(databasePath);
    HttpCacheDatabase::Ptr pCacheDatabase = new HttpCacheDatabase(pCacheDatabaseOpenHelper);

    // set up mock
    SqliteDatabase::Ptr pDb = pCacheDatabaseOpenHelper->getWritableDatabase();
    AutoSqliteDatabase autoSqliteDatabase(pDb);
    ForwardingMockSqliteDatabase::Ptr pMockDatabase =
            new testing::NiceMock<ForwardingMockSqliteDatabase>(databasePath.toString(), pDb);
    pCacheDatabaseOpenHelper->overrideInternalDatabase(pMockDatabase);

    // throw SqlDatabaseCorruptException from deleteRows
    EXPECT_CALL(*pMockDatabase, deleteRows(testing::_, testing::_, testing::_))
            .WillOnce(testing::Throw(SqlDatabaseCorruptException("exception from mock")));

    // When: deleteMetadata
    // Then: throw SqlDatabaseCorruptException
    EASYHTTPCPP_EXPECT_THROW(pCacheDatabase->deleteMetadata(key), SqlDatabaseCorruptException, 100203);
}

} /* namespace test */
} /* namespace easyhttpcpp */
