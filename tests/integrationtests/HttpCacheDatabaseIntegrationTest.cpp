/*
 * Copyright 2017 Sony Corporation
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Timestamp.h"
#include "Poco/Net/HTTPResponse.h"

#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/Headers.h"
#include "HeadersEqualMatcher.h"
#include "EasyHttpCppAssertions.h"
#include "TestDefs.h"
#include "TestFileUtil.h"
#include "TestLogger.h"
#include "TimeInRangeMatcher.h"

#include "HttpIntegrationTestCase.h"
#include "HttpCacheMetadata.h"
#include "HttpCacheDatabase.h"
#include "HttpCacheEnumerationListener.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"
#include "MockHttpCacheEnumerationListener.h"
#include "SynchronizedExecutionRunner.h"

using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::db::SqlExecutionException;
using easyhttpcpp::db::SqlIllegalStateException;
using easyhttpcpp::testutil::TestFileUtil;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "HttpCacheDatabaseIntegrationTest";
static const char* const Key1 = "key1";
static const char* const Key2 = "key2";
static const char* const Key3 = "key3";
static const char* const Test1Url = "http://localhost:9000/test1?a=10";
static const int Test1StatusCode = Poco::Net::HTTPResponse::HTTP_OK;
static const char* const Test1StatusMessage = "Ok";
static const char* const Test1ResponseHeader = EASYHTTPCPP_RAW({"Content-Length":"15","Content-Type":"text/plain"});
static const size_t Test1ResponseBodySize = 15;
static const char* const Test1SentRequestTime = "Fri, 05 Aug 2016 12:00:00 GMT";
static const char* const Test1ReceivedResponseTime = "Fri, 05 Aug 2016 12:00:10 GMT";
static const char* const Test1CreatedMetadataTime = "Fri, 05 Aug 2016 12:00:20 GMT";
static const char* const Test1LastAccessedTime = "Fri, 05 Aug 2016 12:00:30 GMT";

static const char* const Test2Url = "http://localhost:9000/test2?a=10";
static const int Test2StatusCode = Poco::Net::HTTPResponse::HTTP_OK;
static const char* const Test2StatusMessage = "Ok2";
static const char* const Test2ResponseHeader = EASYHTTPCPP_RAW({"Content-Length":"20","Content-Type":"text/plain"});
static const size_t Test2ResponseBodySize = 20;
static const char* const Test2SentRequestTime = "Fri, 05 Aug 2016 13:00:00 GMT";
static const char* const Test2ReceivedResponseTime = "Fri, 05 Aug 2016 13:00:10 GMT";
static const char* const Test2CreatedMetadataTime = "Fri, 05 Aug 2016 13:00:20 GMT";

static const size_t EnumResponseBodySize1 = 100;
static const size_t EnumResponseBodySize2 = 200;
static const size_t EnumResponseBodySize3 = 300;
static const char* const EnumLastAccessedTime1 = "Fri, 05 Aug 2016 14:00:00 GMT";
static const char* const EnumLastAccessedTime2 = "Fri, 05 Aug 2016 14:00:10 GMT";
static const char* const EnumLastAccessedTime3 = "Fri, 05 Aug 2016 14:00:20 GMT";

static const int MultiThreadCount = 10;

namespace {

bool createDefaultCacheRootDir()
{
    return FileUtil::createDirsIfAbsent(HttpTestUtil::getDefaultCacheRootDir());
}

void createDbCacheMetadata(Poco::Path databaseFile, const std::string& key,
        HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata)
{
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));

    pMetadata->setKey(key);
    pMetadata->setUrl(Test1Url);
    pMetadata->setHttpMethod(Request::HttpMethodGet);
    pMetadata->setStatusCode(Test1StatusCode);
    pMetadata->setStatusMessage(Test1StatusMessage);
    pMetadata->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    pMetadata->setResponseBodySize(Test1ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    pMetadata->setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    pMetadata->setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    pMetadata->setCreatedAtEpoch(createdMetadataTime.epochTime());
    Poco::Timestamp lastAccessedTime;
    HttpUtil::tryParseDate(Test1LastAccessedTime, lastAccessedTime);
    pMetadata->setLastAccessedAtEpoch(lastAccessedTime.epochTime());

    db.updateMetadataAll(key, pMetadata);
}

void createDbCacheMetadataWithBodySizeAndLastAccessedTime(Poco::Path databaseFile, const std::string& key,
        size_t responseBodySize, const std::string& lastAccessedTimeStr,
        HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata)
{
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));

    pMetadata->setKey(key);
    pMetadata->setUrl(Test1Url);
    pMetadata->setHttpMethod(Request::HttpMethodGet);
    pMetadata->setStatusCode(Test1StatusCode);
    pMetadata->setStatusMessage(Test1StatusMessage);
    pMetadata->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    pMetadata->setResponseBodySize(responseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    pMetadata->setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    pMetadata->setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    pMetadata->setCreatedAtEpoch(createdMetadataTime.epochTime());
    Poco::Timestamp lastAccessedTime;
    HttpUtil::tryParseDate(lastAccessedTimeStr, lastAccessedTime);
    pMetadata->setLastAccessedAtEpoch(lastAccessedTime.epochTime());

    db.updateMetadataAll(key, pMetadata);
}

void setHttpCacheMetadata(const std::string& key, HttpCacheMetadata::Ptr pHttpCacheMetadata)
{
    pHttpCacheMetadata->setKey(key);
    pHttpCacheMetadata->setUrl(Test1Url);
    pHttpCacheMetadata->setHttpMethod(Request::HttpMethodGet);
    pHttpCacheMetadata->setStatusCode(Test1StatusCode);
    pHttpCacheMetadata->setStatusMessage(Test1StatusMessage);
    pHttpCacheMetadata->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    pHttpCacheMetadata->setResponseBodySize(Test1ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    pHttpCacheMetadata->setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    pHttpCacheMetadata->setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    pHttpCacheMetadata->setCreatedAtEpoch(createdMetadataTime.epochTime());
}

void setHttpCacheMetadataForUpdate(const std::string& key, HttpCacheMetadata::Ptr pHttpCacheMetadata)
{
    pHttpCacheMetadata->setKey(key);
    pHttpCacheMetadata->setUrl(Test2Url);
    pHttpCacheMetadata->setHttpMethod(Request::HttpMethodGet);
    pHttpCacheMetadata->setStatusCode(Test2StatusCode);
    pHttpCacheMetadata->setStatusMessage(Test2StatusMessage);
    pHttpCacheMetadata->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test2ResponseHeader));
    pHttpCacheMetadata->setResponseBodySize(Test2ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test2SentRequestTime, sentRequestTime);
    pHttpCacheMetadata->setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test2ReceivedResponseTime, receivedResponseTime);
    pHttpCacheMetadata->setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test2CreatedMetadataTime, createdMetadataTime);
    pHttpCacheMetadata->setCreatedAtEpoch(createdMetadataTime.epochTime());
}

class DatabaseAccessRunner : public SynchronizedExecutionRunner {
public:
    DatabaseAccessRunner(HttpCacheDatabase& database, const std::string& key) : m_database(database),
            m_key(key)
    {
    }
    const std::string getKey() const
    {
        return m_key;
    }
protected:
    HttpCacheDatabase& m_database;
    const std::string m_key;
};

class GetMetadataExecutionRunner : public DatabaseAccessRunner {
public:
    GetMetadataExecutionRunner(HttpCacheDatabase& database, const std::string& key) :
            DatabaseAccessRunner(database, key)
    {
    }
    bool execute()
    {
        // prepare

        setToReady();
        waitToStart();

        // execute
        m_pCacheMetadata = m_database.getMetadata(m_key);
        return true;
    }
    CacheMetadata::Ptr getCacheMetadata()
    {
        return m_pCacheMetadata;
    }
private:
    CacheMetadata::Ptr m_pCacheMetadata;
};

class DeleteMetadataExecutionRunner : public DatabaseAccessRunner {
public:
    DeleteMetadataExecutionRunner(HttpCacheDatabase& database, const std::string& key) :
            DatabaseAccessRunner(database, key)
    {
    }
    bool execute()
    {
        // prepare

        setToReady();
        waitToStart();

        // execute
        bool ret = m_database.deleteMetadata(m_key);
        return ret;
    }
};

class UpdateMetadataExecutionRunner : public DatabaseAccessRunner {
public:
    UpdateMetadataExecutionRunner(HttpCacheDatabase& database, const std::string& key) :
            DatabaseAccessRunner(database, key)
    {
    }
    bool execute()
    {
        // prepare
        HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
        setHttpCacheMetadataForUpdate(m_key, pHttpCacheMetadata);
        m_pCacheMetadata = pHttpCacheMetadata;

        setToReady();
        waitToStart();

        // execute
        m_database.updateMetadata(m_key, pHttpCacheMetadata);
        return true;
    }
    CacheMetadata::Ptr getCacheMetadata()
    {
        return m_pCacheMetadata;
    }
private:
    CacheMetadata::Ptr m_pCacheMetadata;
};

class UpdateLastAccessedSecExecutionRunner : public DatabaseAccessRunner {
public:
    UpdateLastAccessedSecExecutionRunner(HttpCacheDatabase& database, const std::string& key) :
            DatabaseAccessRunner(database, key)
    {
    }
    bool execute()
    {
        // prepare

        setToReady();
        waitToStart();

        // execute
        bool ret = m_database.updateLastAccessedSec(m_key);
        return ret;
    }
};

} /* namespace */

class HttpCacheDatabaseIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        TestFileUtil::setFullAccess(path);
        FileUtil::removeDirsIfPresent(path);

        EASYHTTPCPP_TESTLOG_SETUP_END();
    }

    Poco::AutoPtr<DatabaseAccessRunner> m_pExecutes[MultiThreadCount];
};

// getMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsFalse_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: getMetadata by not exist key
    // Then: return NULL
    HttpCacheMetadata::Ptr pHttpCacheMetadata = database.getMetadata(Key1);
    EXPECT_TRUE(pHttpCacheMetadata.isNull());
}

// getMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsTrue_WhenExistKey)
{
    // Given: insert cacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadata);

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: getMetadata by exist key
    // Then: return metadata
    HttpCacheMetadata::Ptr pHttpCacheMetadata = database.getMetadata(Key1);
    EXPECT_FALSE(pHttpCacheMetadata.isNull());
    CacheMetadata::Ptr pCacheMetadata = pHttpCacheMetadata;

    // check metadata
    EXPECT_EQ(pMetadata->getKey(), pHttpCacheMetadata->getKey());
    EXPECT_EQ(pMetadata->getUrl(), pHttpCacheMetadata->getUrl());
    EXPECT_EQ(pMetadata->getHttpMethod(), pHttpCacheMetadata->getHttpMethod());
    EXPECT_EQ(pMetadata->getStatusCode(), pHttpCacheMetadata->getStatusCode());
    EXPECT_EQ(pMetadata->getStatusMessage(), pHttpCacheMetadata->getStatusMessage());
    EXPECT_THAT(pHttpCacheMetadata->getResponseHeaders(), testutil::equalHeaders(pMetadata->getResponseHeaders()));
    EXPECT_EQ(pMetadata->getResponseBodySize(), pHttpCacheMetadata->getResponseBodySize());
    EXPECT_EQ(pMetadata->getSentRequestAtEpoch(), pHttpCacheMetadata->getSentRequestAtEpoch());
    EXPECT_EQ(pMetadata->getReceivedResponseAtEpoch(), pHttpCacheMetadata->getReceivedResponseAtEpoch());
    EXPECT_EQ(pMetadata->getCreatedAtEpoch(), pHttpCacheMetadata->getCreatedAtEpoch());
}

// getMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ThrowsSqlIllegalStateException_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: getMetadata by not exist key
    // Then: throw SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.getMetadata(Key1), SqlIllegalStateException, 100201);
}

// deleteMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsFalse_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: deleteMetadata by not exist key
    // Then: return false
    EXPECT_FALSE(database.deleteMetadata(Key1));
}

// deletMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsTrue_WhenExistKey)
{
    // Given: insert cacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadata);

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: deleteMetadata by exist key
    // Then: return true and remove metadata from database
    EXPECT_TRUE(database.deleteMetadata(Key1));

    // check deleted metadata from database
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    EXPECT_TRUE(db.getMetadataAll(Key1).isNull());
}

// deleteMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ThrowsSqlIllegalStateException_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: deleteMetadata by not exist key
    // Then: throw SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.deleteMetadata(Key1), SqlIllegalStateException, 100201);
}

// deletMetadata
TEST_F(HttpCacheDatabaseIntegrationTest,
        deleteMetadata_ThrowsSqlException_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadata);

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: deleteMetadata by exist key
    // Then: throw SqlException

    // database file を read only にして database にアクセスしたとき、
    // Windows では SqliteDatabase の constructor の new Poco::Data::Session で Poco::Data::ConnectionFailedException
    // が発生して easyhttpcpp::db::SqlIllegalStateException が throw されます。
    // Linux では new Poco::Data::Session ではエラーとならず、Poco::Data::Statement::execute で
    // Poco::Data::SQLite::ReadOnlyException が発生して easyhttpcpp::db::SqlExecutionException が throw されます。
    // このため、このテストでは、_WIN32 でチェックする Exception を切り替えています。
#ifdef _WIN32
    // throw SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.deleteMetadata(Key1), SqlIllegalStateException, 100201);
#else
    // throw SqlExecutionException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.deleteMetadata(Key1), SqlExecutionException, 100202);
#endif
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_InsertsMetadata_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
    setHttpCacheMetadata(Key1, pHttpCacheMetadata);

    // When: updateMetadata by not exist key
    // Then: insert metadata in database
    Poco::Timestamp startTime;
    database.updateMetadata(Key1, pHttpCacheMetadata);
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata;
    pMetadata = db.getMetadataAll(Key1);
    ASSERT_FALSE(pMetadata.isNull());
    EXPECT_EQ(pHttpCacheMetadata->getKey(), pMetadata->getKey());
    EXPECT_EQ(pHttpCacheMetadata->getUrl(), pMetadata->getUrl());
    EXPECT_EQ(pHttpCacheMetadata->getHttpMethod(), pMetadata->getHttpMethod());
    EXPECT_EQ(pHttpCacheMetadata->getStatusCode(), pMetadata->getStatusCode());
    EXPECT_EQ(pHttpCacheMetadata->getStatusMessage(), pMetadata->getStatusMessage());
    EXPECT_THAT(pMetadata->getResponseHeaders(), testutil::equalHeaders(pHttpCacheMetadata->getResponseHeaders()));
    EXPECT_EQ(pHttpCacheMetadata->getResponseBodySize(), pMetadata->getResponseBodySize());
    EXPECT_EQ(pHttpCacheMetadata->getSentRequestAtEpoch(), pMetadata->getSentRequestAtEpoch());
    EXPECT_EQ(pHttpCacheMetadata->getReceivedResponseAtEpoch(), pMetadata->getReceivedResponseAtEpoch());
    EXPECT_EQ(pHttpCacheMetadata->getCreatedAtEpoch(), pMetadata->getCreatedAtEpoch());
    EXPECT_THAT(pMetadata->getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_UpdatesMatadata_WhenExistKey)
{
    // Given: insert CacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCreate = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadataForCreate);

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
    setHttpCacheMetadataForUpdate(Key1, pHttpCacheMetadata);

    // When: updateMetadata by exist key
    // Then: update metadata in database
    Poco::Timestamp startTime;
    database.updateMetadata(Key1, pHttpCacheMetadata);
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata;
    pMetadata = db.getMetadataAll(Key1);
    ASSERT_FALSE(pMetadata.isNull());
    EXPECT_EQ(pHttpCacheMetadata->getKey(), pMetadata->getKey());
    EXPECT_EQ(pHttpCacheMetadata->getUrl(), pMetadata->getUrl());
    EXPECT_EQ(pHttpCacheMetadata->getHttpMethod(), pMetadata->getHttpMethod());
    EXPECT_EQ(pHttpCacheMetadata->getStatusCode(), pMetadata->getStatusCode());
    EXPECT_EQ(pHttpCacheMetadata->getStatusMessage(), pMetadata->getStatusMessage());
    EXPECT_THAT(pMetadata->getResponseHeaders(), testutil::equalHeaders(pHttpCacheMetadata->getResponseHeaders()));
    EXPECT_EQ(pHttpCacheMetadata->getResponseBodySize(), pMetadata->getResponseBodySize());
    EXPECT_EQ(pHttpCacheMetadata->getSentRequestAtEpoch(), pMetadata->getSentRequestAtEpoch());
    EXPECT_EQ(pHttpCacheMetadata->getReceivedResponseAtEpoch(), pMetadata->getReceivedResponseAtEpoch());
    EXPECT_EQ(pHttpCacheMetadata->getCreatedAtEpoch(), pMetadata->getCreatedAtEpoch());
    EXPECT_THAT(pMetadata->getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ThrowsSqlIllegalStateException_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
    setHttpCacheMetadata(Key1, pHttpCacheMetadata);

    // When: updateMetadata by not exist key
    // Then: throws SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateMetadata(Key1, pHttpCacheMetadata), SqlIllegalStateException, 100201);
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest,
        updateMetadata_ThrowsSqlException_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCreate = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadataForCreate);

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
    setHttpCacheMetadataForUpdate(Key1, pHttpCacheMetadata);

    // When: updateMetadata by exist key
    // Then: throws SqlException

    // database file を read only にして database にアクセスしたとき、
    // Windows では SqliteDatabase の constructor の new Poco::Data::Session で Poco::Data::ConnectionFailedException
    // が発生して easyhttpcpp::db::SqlIllegalStateException が throw されます。
    // Linux では new Poco::Data::Session ではエラーとならず、Poco::Data::Statement::execute で
    // Poco::Data::SQLite::ReadOnlyException が発生して easyhttpcpp::db::SqlExecutionException が throw されます。
    // このため、このテストでは、_WIN32 でチェックする Exception を切り替えています。
#ifdef _WIN32
    // throws SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateMetadata(Key1, pHttpCacheMetadata), SqlIllegalStateException, 100201);
#else
    // throws SqlExecutionException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateMetadata(Key1, pHttpCacheMetadata), SqlExecutionException, 100202);
#endif
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsFalse_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: updateLastAccessedSec by not exist key
    // Then: return false
    EXPECT_FALSE(database.updateLastAccessedSec(Key1));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsNormally_WhenExistKey)
{
    // Given: insert cacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCreate = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadataForCreate);

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));
    Poco::Thread::sleep(1500);  // wait 1.5 sec. (for confirm to update lastAccessedSec))

    // When: updateLastAccessedSec by exist key
    // Then: return normally and update lastAccessedSec
    Poco::Timestamp startTime;
    EXPECT_TRUE(database.updateLastAccessedSec(Key1));
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata;
    pMetadata = db.getMetadataAll(Key1);
    ASSERT_FALSE(pMetadata.isNull());
    EXPECT_THAT(pMetadata->getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ThrowsSqlIllegalStateException_WhenCannotOopenDatabase)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: updateLastAccessedSec by not exist key
    // Then: throws SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateLastAccessedSec(Key1), SqlIllegalStateException, 100201);
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest,
        updateLastAccessedSec_ThrowsSqlException_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCreate = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadata(databaseFile, Key1, pMetadataForCreate);

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    // When: updateLastAccessedSec by exist key
    // Then: throws SqlException

    // database file を read only にして database にアクセスしたとき、
    // Windows では SqliteDatabase の constructor の new Poco::Data::Session で Poco::Data::ConnectionFailedException
    // が発生して easyhttpcpp::db::SqlIllegalStateException が throw されます。
    // Linux では new Poco::Data::Session ではエラーとならず、Poco::Data::Statement::execute で
    // Poco::Data::SQLite::ReadOnlyException が発生して easyhttpcpp::db::SqlExecutionException が throw されます。
    // このため、このテストでは、_WIN32 でチェックする Exception を切り替えています。
#ifdef _WIN32
    // throws SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateLastAccessedSec(Key1), SqlIllegalStateException, 100201);
#else
    // throws SqlExecutionException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.updateLastAccessedSec(Key1), SqlExecutionException, 100202);
#endif
}

// enumerate
TEST_F(HttpCacheDatabaseIntegrationTest, enumerate_ReturnsNormallyAndNoEnumerate_WhenDatabaseIsEmpty)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(testing::_)).Times(0);

    // When: enumerate
    // Then: return normally
    database.enumerate(&mockHttpCacheEnumerationListener);
}

namespace {

class EnumerationParamMatcher :
        public testing::MatcherInterface<const HttpCacheEnumerationListener::EnumerationParam&> {
public:
    explicit EnumerationParamMatcher(const std::string& key, size_t size) : m_key(key), m_size(size)
    {
    }
    virtual bool MatchAndExplain(const HttpCacheEnumerationListener::EnumerationParam& param,
            testing::MatchResultListener* listener) const
    {
        if (param.m_key == m_key && param.m_responseBodySize == m_size)
        {
            return true;
        }
        *listener << StringUtil::format("key=%s size=%zu", param.m_key.c_str(), param.m_responseBodySize);
        return false;
    }

    virtual void DescribeTo(::std::ostream* os) const
    {
        *os << StringUtil::format("is equal to key=%s size=%zu", m_key.c_str(), m_size);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const
    {
        *os << StringUtil::format("is not equal to key=%s size=%zu", m_key.c_str(), m_size);
    }

private:
    const std::string m_key;
    const size_t m_size;
};

inline testing::Matcher<const HttpCacheEnumerationListener::EnumerationParam&> equalsEnumerationParam(
        const std::string& key, size_t size) {
  return MakeMatcher(new EnumerationParamMatcher(key, size));
}

}   /* namespace */

// enumerate
TEST_F(HttpCacheDatabaseIntegrationTest,
        enumerate_ReturnsNormallyAndEnumerateByOrderOfLastAccessedTime_WhenDatabaseExistsMedatada)
{
    // Given: insert CacheMetadata in database (lastAccessTime old -> new : Key3 -> Key1 -> Key2))
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata1 = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key1,
            EnumResponseBodySize1, EnumLastAccessedTime2, pMetadata1);
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata2 = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key2,
            EnumResponseBodySize2, EnumLastAccessedTime3, pMetadata2);
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata3 = new HttpCacheDatabase::HttpCacheMetadataAll();
    createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key3,
            EnumResponseBodySize3, EnumLastAccessedTime1, pMetadata3);

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    testing::InSequence seq;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key3, EnumResponseBodySize3))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key1, EnumResponseBodySize1))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key2, EnumResponseBodySize2))).
            WillOnce(testing::Return(true));

    // When: enumerate
    // Then: return normally and onEnumerate will be called by order of lastAccessedTime.
    database.enumerate(&mockHttpCacheEnumerationListener);
}

// enumerate
TEST_F(HttpCacheDatabaseIntegrationTest, enumerate_ThrowsSqlIllegalStateException_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(testing::_)).Times(0);

    // When: enumerate
    // Then: throws SqlIllegalStateException
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(database.enumerate(&mockHttpCacheEnumerationListener),
            SqlIllegalStateException, 100201);
}

// getMetadata (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsTrueAndGetMetadata_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call getMetadata.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    for (int i = 0; i < MultiThreadCount; i++) {
        createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), pMetadata);
    }

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i] = new GetMetadataExecutionRunner(database, StringUtil::format("Key%d", i));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutes[i]);
        m_pExecutes[i]->waitToReady();
    }

    // When: start getMetadata
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: getMetadata succeeded
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        CacheMetadata::Ptr pCacheMetadata = static_cast<GetMetadataExecutionRunner*>
                (m_pExecutes[i].get())->getCacheMetadata();
        EXPECT_FALSE(pCacheMetadata.isNull()) << index;
        if (!pCacheMetadata.isNull()) {
            EXPECT_EQ(m_pExecutes[i]->getKey(), pCacheMetadata->getKey()) << index;
        }
    }
}

// deleteMetadata (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsTrueAndDeleteMetadata_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call deleteMetadata.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    for (int i = 0; i < MultiThreadCount; i++) {
        createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), pMetadata);
    }

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i] = new DeleteMetadataExecutionRunner(database, StringUtil::format("Key%d", i));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutes[i]);
        m_pExecutes[i]->waitToReady();
    }

    // When: start deleteMetadata
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: deleteMetadata succeeded
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        EXPECT_TRUE(db.getMetadataAll(m_pExecutes[i]->getKey()).isNull()) << index;
    }
}

// updateMetadata (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsTrueAndUpdateMetadata_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call updateMetadata.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    for (int i = 0; i < MultiThreadCount; i++) {
        createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), pMetadata);
    }

    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i] = new UpdateMetadataExecutionRunner(database, StringUtil::format("Key%d", i));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutes[i]);
        m_pExecutes[i]->waitToReady();
    }

    // When: start updateMetadata
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i]->setToStart();
    }

    threadPool.joinAll();

    // Then: updateMetadata succeeded
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCheck;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        pMetadataForCheck = db.getMetadataAll(m_pExecutes[i]->getKey());
        ASSERT_FALSE(pMetadataForCheck.isNull()) << index;
        CacheMetadata::Ptr pCacheMetadata = static_cast<UpdateMetadataExecutionRunner*>
                (m_pExecutes[i].get())->getCacheMetadata();
        EXPECT_FALSE(pCacheMetadata.isNull()) << index;
        if (!pCacheMetadata.isNull()) {
            HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*>(pCacheMetadata.get());
            EXPECT_EQ(m_pExecutes[i]->getKey(), pMetadataForCheck->getKey()) << index;
            EXPECT_EQ(pHttpCacheMetadata->getUrl(), pMetadataForCheck->getUrl()) << index;
        }
    }
}

// updateLaastAccessedSec (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest,
        updateLastAccessedSec_ReturnsTrueAndUpdateLastAccessedSec_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call updateLastAccessSec.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadata = new HttpCacheDatabase::HttpCacheMetadataAll();
    for (int i = 0; i < MultiThreadCount; i++) {
        createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), pMetadata);
    }

    Poco::Thread::sleep(1500);  // wait 1.5 sec. (for confirm to update lastAccessedSec))
    HttpCacheDatabase database(new HttpCacheDatabaseOpenHelper(databaseFile));

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i] = new UpdateLastAccessedSecExecutionRunner(database, StringUtil::format("Key%d", i));
    }

    // execute until ready
    for (int i = 0; i < MultiThreadCount; i++) {
        threadPool.start(*m_pExecutes[i]);
        m_pExecutes[i]->waitToReady();
    }

    Poco::Timestamp startTime;
    // When: start updateLastAccessedSec
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pExecutes[i]->setToStart();
    }

    threadPool.joinAll();
    Poco::Timestamp endTime;

    // Then: updateLastAccessedSec succeeded
    HttpCacheDatabase db(new HttpCacheDatabaseOpenHelper(databaseFile));
    HttpCacheDatabase::HttpCacheMetadataAll::Ptr pMetadataForCheck;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        pMetadataForCheck = db.getMetadataAll(m_pExecutes[i]->getKey());
        ASSERT_FALSE(pMetadataForCheck.isNull()) << index;
        EXPECT_THAT(pMetadataForCheck->getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
                endTime.epochTime()));
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
