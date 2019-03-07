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
#include "easyhttpcpp/Headers.h"
#include "HeadersEqualMatcher.h"
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

bool createDbCacheMetadata(Poco::Path databaseFile, const std::string& key,
        HttpCacheDatabase::HttpCacheMetadataAll& metadata)
{
    HttpCacheDatabase db(databaseFile);

    metadata.setKey(key);
    metadata.setUrl(Test1Url);
    metadata.setHttpMethod(Request::HttpMethodGet);
    metadata.setStatusCode(Test1StatusCode);
    metadata.setStatusMessage(Test1StatusMessage);
    metadata.setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    metadata.setResponseBodySize(Test1ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    metadata.setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    metadata.setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    metadata.setCreatedAtEpoch(createdMetadataTime.epochTime());
    Poco::Timestamp lastAccessedTime;
    HttpUtil::tryParseDate(Test1LastAccessedTime, lastAccessedTime);
    metadata.setLastAccessedAtEpoch(lastAccessedTime.epochTime());

    return db.updateMetadataAll(key, metadata);
}

bool createDbCacheMetadataWithBodySizeAndLastAccessedTime(Poco::Path databaseFile, const std::string& key,
        size_t responseBodySize, const std::string& lastAccessedTimeStr,
        HttpCacheDatabase::HttpCacheMetadataAll& metadata)
{
    HttpCacheDatabase db(databaseFile);

    metadata.setKey(key);
    metadata.setUrl(Test1Url);
    metadata.setHttpMethod(Request::HttpMethodGet);
    metadata.setStatusCode(Test1StatusCode);
    metadata.setStatusMessage(Test1StatusMessage);
    metadata.setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    metadata.setResponseBodySize(responseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    metadata.setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    metadata.setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    metadata.setCreatedAtEpoch(createdMetadataTime.epochTime());
    Poco::Timestamp lastAccessedTime;
    HttpUtil::tryParseDate(lastAccessedTimeStr, lastAccessedTime);
    metadata.setLastAccessedAtEpoch(lastAccessedTime.epochTime());

    return db.updateMetadataAll(key, metadata);
}

void setHttpCacheMetadata(const std::string& key, HttpCacheMetadata& httpCacheMetadata)
{
    httpCacheMetadata.setKey(key);
    httpCacheMetadata.setUrl(Test1Url);
    httpCacheMetadata.setHttpMethod(Request::HttpMethodGet);
    httpCacheMetadata.setStatusCode(Test1StatusCode);
    httpCacheMetadata.setStatusMessage(Test1StatusMessage);
    httpCacheMetadata.setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test1ResponseHeader));
    httpCacheMetadata.setResponseBodySize(Test1ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test1SentRequestTime, sentRequestTime);
    httpCacheMetadata.setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test1ReceivedResponseTime, receivedResponseTime);
    httpCacheMetadata.setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, createdMetadataTime);
    httpCacheMetadata.setCreatedAtEpoch(createdMetadataTime.epochTime());
}

void setHttpCacheMetadataForUpdate(const std::string& key, HttpCacheMetadata& httpCacheMetadata)
{
    httpCacheMetadata.setKey(key);
    httpCacheMetadata.setUrl(Test2Url);
    httpCacheMetadata.setHttpMethod(Request::HttpMethodGet);
    httpCacheMetadata.setStatusCode(Test2StatusCode);
    httpCacheMetadata.setStatusMessage(Test2StatusMessage);
    httpCacheMetadata.setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(Test2ResponseHeader));
    httpCacheMetadata.setResponseBodySize(Test2ResponseBodySize);
    Poco::Timestamp sentRequestTime;
    HttpUtil::tryParseDate(Test2SentRequestTime, sentRequestTime);
    httpCacheMetadata.setSentRequestAtEpoch(sentRequestTime.epochTime());
    Poco::Timestamp receivedResponseTime;
    HttpUtil::tryParseDate(Test2ReceivedResponseTime, receivedResponseTime);
    httpCacheMetadata.setReceivedResponseAtEpoch(receivedResponseTime.epochTime());
    Poco::Timestamp createdMetadataTime;
    HttpUtil::tryParseDate(Test2CreatedMetadataTime, createdMetadataTime);
    httpCacheMetadata.setCreatedAtEpoch(createdMetadataTime.epochTime());
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
        HttpCacheMetadata* pHttpCacheMetadata;
        bool ret = m_database.getMetadata(m_key, pHttpCacheMetadata);
        m_pCacheMetadata = pHttpCacheMetadata;
        return ret;
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
        HttpCacheMetadata* pHttpCacheMetadata = new HttpCacheMetadata();
        m_pCacheMetadata = pHttpCacheMetadata;
        setHttpCacheMetadataForUpdate(m_key, *pHttpCacheMetadata);

        setToReady();
        waitToStart();

        // execute
        bool ret = m_database.updateMetadata(m_key, pHttpCacheMetadata);
        return ret;
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
    HttpCacheDatabase database(databaseFile);

    // When: getMetadata by not exist key
    // Then: return false
    HttpCacheMetadata* pHttpCacheMetadata;
    EXPECT_FALSE(database.getMetadata(Key1, pHttpCacheMetadata));
}

// getMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsTrue_WhenExistKey)
{
    // Given: insert cacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadata));

    HttpCacheDatabase database(databaseFile);

    // When: getMetadata by exist key
    // Then: return true and get metadata
    HttpCacheMetadata* pHttpCacheMetadata;
    EXPECT_TRUE(database.getMetadata(Key1, pHttpCacheMetadata));
    CacheMetadata::Ptr pCacheMetadata = pHttpCacheMetadata;

    // check metadata
    EXPECT_EQ(metadata.getKey(), pHttpCacheMetadata->getKey());
    EXPECT_EQ(metadata.getUrl(), pHttpCacheMetadata->getUrl());
    EXPECT_EQ(metadata.getHttpMethod(), pHttpCacheMetadata->getHttpMethod());
    EXPECT_EQ(metadata.getStatusCode(), pHttpCacheMetadata->getStatusCode());
    EXPECT_EQ(metadata.getStatusMessage(), pHttpCacheMetadata->getStatusMessage());
    EXPECT_THAT(pHttpCacheMetadata->getResponseHeaders(), testutil::equalHeaders(metadata.getResponseHeaders()));
    EXPECT_EQ(metadata.getResponseBodySize(), pHttpCacheMetadata->getResponseBodySize());
    EXPECT_EQ(metadata.getSentRequestAtEpoch(), pHttpCacheMetadata->getSentRequestAtEpoch());
    EXPECT_EQ(metadata.getReceivedResponseAtEpoch(), pHttpCacheMetadata->getReceivedResponseAtEpoch());
    EXPECT_EQ(metadata.getCreatedAtEpoch(), pHttpCacheMetadata->getCreatedAtEpoch());
}

// getMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsFalse_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    // When: getMetadata by not exist key
    // Then: return false
    HttpCacheMetadata* pHttpCacheMetadata;
    EXPECT_FALSE(database.getMetadata(Key1, pHttpCacheMetadata));
}

// deleteMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsFalse_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

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
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadata));

    HttpCacheDatabase database(databaseFile);

    // When: deleteMetadata by exist key
    // Then: return true and remove metadata from database
    EXPECT_TRUE(database.deleteMetadata(Key1));

    // check deleted metadata from database
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadataWork;
    EXPECT_FALSE(db.getMetadataAll(Key1, metadataWork));
}

// deleteMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsFalse_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    // When: deleteMetadata by not exist key
    // Then: return false
    EXPECT_FALSE(database.deleteMetadata(Key1));
}

// deletMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, deleteMetadata_ReturnsFalse_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadata));

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(databaseFile);

    // When: deleteMetadata by exist key
    // Then: return false
    EXPECT_FALSE(database.deleteMetadata(Key1));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsTrue_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    HttpCacheMetadata httpCacheMetadata;
    setHttpCacheMetadata(Key1, httpCacheMetadata);

    // When: updateMetadata by not exist key
    // Then: return true and update metadata
    Poco::Timestamp startTime;
    EXPECT_TRUE(database.updateMetadata(Key1, &httpCacheMetadata));
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    ASSERT_TRUE(db.getMetadataAll(Key1, metadata));
    EXPECT_EQ(httpCacheMetadata.getKey(), metadata.getKey());
    EXPECT_EQ(httpCacheMetadata.getUrl(), metadata.getUrl());
    EXPECT_EQ(httpCacheMetadata.getHttpMethod(), metadata.getHttpMethod());
    EXPECT_EQ(httpCacheMetadata.getStatusCode(), metadata.getStatusCode());
    EXPECT_EQ(httpCacheMetadata.getStatusMessage(), metadata.getStatusMessage());
    EXPECT_THAT(metadata.getResponseHeaders(), testutil::equalHeaders(httpCacheMetadata.getResponseHeaders()));
    EXPECT_EQ(httpCacheMetadata.getResponseBodySize(), metadata.getResponseBodySize());
    EXPECT_EQ(httpCacheMetadata.getSentRequestAtEpoch(), metadata.getSentRequestAtEpoch());
    EXPECT_EQ(httpCacheMetadata.getReceivedResponseAtEpoch(), metadata.getReceivedResponseAtEpoch());
    EXPECT_EQ(httpCacheMetadata.getCreatedAtEpoch(), metadata.getCreatedAtEpoch());
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsTrue_WhenExistKey)
{
    // Given: insert CacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCreate;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadataForCreate));

    HttpCacheDatabase database(databaseFile);

    HttpCacheMetadata httpCacheMetadata;
    setHttpCacheMetadataForUpdate(Key1, httpCacheMetadata);

    // When: updateMetadata by exist key
    // Then: return true and update metadate
    Poco::Timestamp startTime;
    EXPECT_TRUE(database.updateMetadata(Key1, &httpCacheMetadata));
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    EXPECT_TRUE(db.getMetadataAll(Key1, metadata));
    EXPECT_EQ(httpCacheMetadata.getKey(), metadata.getKey());
    EXPECT_EQ(httpCacheMetadata.getUrl(), metadata.getUrl());
    EXPECT_EQ(httpCacheMetadata.getHttpMethod(), metadata.getHttpMethod());
    EXPECT_EQ(httpCacheMetadata.getStatusCode(), metadata.getStatusCode());
    EXPECT_EQ(httpCacheMetadata.getStatusMessage(), metadata.getStatusMessage());
    EXPECT_THAT(metadata.getResponseHeaders(), testutil::equalHeaders(httpCacheMetadata.getResponseHeaders()));
    EXPECT_EQ(httpCacheMetadata.getResponseBodySize(), metadata.getResponseBodySize());
    EXPECT_EQ(httpCacheMetadata.getSentRequestAtEpoch(), metadata.getSentRequestAtEpoch());
    EXPECT_EQ(httpCacheMetadata.getReceivedResponseAtEpoch(), metadata.getReceivedResponseAtEpoch());
    EXPECT_EQ(httpCacheMetadata.getCreatedAtEpoch(), metadata.getCreatedAtEpoch());
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsFalse_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    HttpCacheMetadata httpCacheMetadata;
    setHttpCacheMetadata(Key1, httpCacheMetadata);

    // When: updateMetadata by not exist key
    // Then: return false
    EXPECT_FALSE(database.updateMetadata(Key1, &httpCacheMetadata));
}

// updateMetadata
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsFalse_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCreate;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadataForCreate));

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(databaseFile);

    HttpCacheMetadata httpCacheMetadata;
    setHttpCacheMetadataForUpdate(Key1, httpCacheMetadata);

    // When: updateMetadata by exist key
    // Then: return false
    EXPECT_FALSE(database.updateMetadata(Key1, &httpCacheMetadata));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsFalse_WhenNotExistKey)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    // When: deleteMetadata by not exist key
    // Then: return false
    EXPECT_FALSE(database.updateLastAccessedSec(Key1));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsTrue_WhenExistKey)
{
    // Given: insert cacheMetadata in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCreate;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadataForCreate));

    HttpCacheDatabase database(databaseFile);
    Poco::Thread::sleep(1500);  // wait 1.5 sec. (for confirm to update lastAccessedSec))

    // When: updateMetadata by exist key
    // Then: return true and update lastAccessedSec
    Poco::Timestamp startTime;
    EXPECT_TRUE(database.updateLastAccessedSec(Key1));
    Poco::Timestamp endTime;

    // check updated metadata
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    EXPECT_TRUE(db.getMetadataAll(Key1, metadata));
    EXPECT_THAT(metadata.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
            endTime.epochTime()));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsFalse_WhenCannotOopenDatabase)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    // When: deleteMetadata by not exist key
    // Then: return false
    EXPECT_FALSE(database.updateLastAccessedSec(Key1));
}

// updateLastAccessedSec
TEST_F(HttpCacheDatabaseIntegrationTest, updateLastAccessedSec_ReturnsFalse_WhenCannotWriteDatabase)
{
    // Given: set readonly to database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCreate;
    ASSERT_TRUE(createDbCacheMetadata(databaseFile, Key1, metadataForCreate));

    Poco::File file(HttpTestUtil::getDefaultCacheDatabaseFile());
    TestFileUtil::setReadOnly(file.path());

    HttpCacheDatabase database(databaseFile);

    // When: updateMetadata by exist key
    // Then: return false
    EXPECT_FALSE(database.updateLastAccessedSec(Key1));
}

// enumerate
TEST_F(HttpCacheDatabaseIntegrationTest, enumerate_ReturnsTrueAndNoEnumerate_WhenDatabaseIsEmpty)
{
    // Given: no key in database
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(testing::_)).Times(0);

    // When: enumerate
    // Then: return true
    EXPECT_TRUE(database.enumerate(&mockHttpCacheEnumerationListener));
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
        enumerate_ReturnsTrueAndEnumerateByOrderOfLastAccessedTime_WhenDatabaseExistsMedatada)
{
    // Given: insert CacheMetadata in database (lastAccessTime old -> new : Key3 -> Key1 -> Key2))
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    ASSERT_TRUE(createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key1,
            EnumResponseBodySize1, EnumLastAccessedTime2, metadata1));
    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    ASSERT_TRUE(createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key2,
            EnumResponseBodySize2, EnumLastAccessedTime3, metadata2));
    HttpCacheDatabase::HttpCacheMetadataAll metadata3;
    ASSERT_TRUE(createDbCacheMetadataWithBodySizeAndLastAccessedTime(databaseFile, Key3,
            EnumResponseBodySize3, EnumLastAccessedTime1, metadata3));

    HttpCacheDatabase database(databaseFile);

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    testing::InSequence seq;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key3, EnumResponseBodySize3))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key1, EnumResponseBodySize1))).
            WillOnce(testing::Return(true));
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(equalsEnumerationParam(Key2, EnumResponseBodySize2))).
            WillOnce(testing::Return(true));

    // When: enumerate
    // Then: return true and onEnumerate will be called by order of lastAccessedTime.
    EXPECT_TRUE(database.enumerate(&mockHttpCacheEnumerationListener));
}

// enumerate
TEST_F(HttpCacheDatabaseIntegrationTest, enumerate_ReturnsFalse_WhenDatabaseDirectoryIsAbsent)
{
    // Given: do not create database directory
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase database(databaseFile);

    MockHttpCacheEnumerationListener mockHttpCacheEnumerationListener;
    EXPECT_CALL(mockHttpCacheEnumerationListener, onEnumerate(testing::_)).Times(0);

    // When: enumerate
    // Then: return false
    EXPECT_FALSE(database.enumerate(&mockHttpCacheEnumerationListener));
}

// getMetadata (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest, getMetadata_ReturnsTrueAndGetMetadata_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call getMetadata.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), metadata));
    }

    HttpCacheDatabase database(databaseFile);

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
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), metadata));
    }

    HttpCacheDatabase database(databaseFile);

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
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCheck;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        EXPECT_FALSE(db.getMetadataAll(m_pExecutes[i]->getKey(), metadataForCheck)) << index;
    }
}

// updateMetadata (multi thread))
TEST_F(HttpCacheDatabaseIntegrationTest, updateMetadata_ReturnsTrueAndUpdateMetadata_WhenCalledOnMultiThread)
{
    // Given: insert CacheMetadata in database. each thread wait before call updateMetadata.
    ASSERT_TRUE(createDefaultCacheRootDir()) << "cannot create cache root directory.";
    Poco::Path databaseFile(HttpTestUtil::getDefaultCacheDatabaseFile());
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), metadata));
    }

    HttpCacheDatabase database(databaseFile);

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
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCheck;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        EXPECT_TRUE(db.getMetadataAll(m_pExecutes[i]->getKey(), metadataForCheck)) << index;
        CacheMetadata::Ptr pCacheMetadata = static_cast<UpdateMetadataExecutionRunner*>
                (m_pExecutes[i].get())->getCacheMetadata();
        EXPECT_FALSE(pCacheMetadata.isNull()) << index;
        if (!pCacheMetadata.isNull()) {
            HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*>(pCacheMetadata.get());
            EXPECT_EQ(m_pExecutes[i]->getKey(), metadataForCheck.getKey()) << index;
            EXPECT_EQ(pHttpCacheMetadata->getUrl(), metadataForCheck.getUrl()) << index;
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
    HttpCacheDatabase::HttpCacheMetadataAll metadata;
    for (int i = 0; i < MultiThreadCount; i++) {
        ASSERT_TRUE(createDbCacheMetadata(databaseFile, StringUtil::format("Key%d", i), metadata));
    }

    Poco::Thread::sleep(1500);  // wait 1.5 sec. (for confirm to update lastAccessedSec))
    HttpCacheDatabase database(databaseFile);

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
    HttpCacheDatabase db(databaseFile);
    HttpCacheDatabase::HttpCacheMetadataAll metadataForCheck;
    for (int i = 0; i < MultiThreadCount; i++) {
        std::string index = StringUtil::format("[%d]", i);
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << index;
        EXPECT_TRUE(db.getMetadataAll(m_pExecutes[i]->getKey(), metadataForCheck)) << index;
        EXPECT_THAT(metadataForCheck.getLastAccessedAtEpoch(), testutil::isTimeInRange(startTime.epochTime(),
                endTime.epochTime()));
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
