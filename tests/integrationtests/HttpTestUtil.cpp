/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Path.h"
#include "Poco/URI.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"

#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpsTestServer;

namespace easyhttpcpp {
namespace test {

static const char* const DefaultCacheParentPath = "/";
static const char* const DefaultCachePath = "/HttpCache/";
static const char* const CacheRootDir = "cache/";
static const char* const CacheTempDir = "cache/temp/";
static const char* const CacheDatabaseFile = "cache/cache_metadata.db";
static const char* const CacheDir = "cache/";
static const char* const DefaultCertRootDir = "/cert/";
static const char* const DefaultCertRootParentDir = "/";
static const char* const TestDataForValidCert = "/HttpIntegrationTest/02_https_localhost_cert/cert/";
static const char* const ValidRootCaDir = "client/rootCa/";
static const char* const ServerCertFile = "server/server.pem";

static const char* const DatabaseFileName = "cache_metadata.db";

static const Request::HttpMethod Test1HttpMethod = Request::HttpMethodGet;
static const int Test1StatusCode = Poco::Net::HTTPResponse::HTTP_OK;
static const char* const Test1StatusMessage = "OK";
static const char* const Test1Header1 = "x-header1";
static const char* const Test1HeaderValue1 = "x-value1";
static const char* const Test1SentRequestTime = "Fri, 05 Aug 2016 12:00:00 GMT";
static const char* const Test1ReceiveResponseTime = "Fri, 05 Aug 2016 12:00:10 GMT";
static const char* const Test1CreatedMetadataTime = "Fri, 05 Aug 2016 12:00:20 GMT";

std::string HttpTestUtil::getDefaultCacheParentPath()
{
    return Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCacheParentPath).toString();
}

std::string HttpTestUtil::getDefaultCachePath()
{
    return Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCachePath).toString();
}

std::string HttpTestUtil::getDefaultCacheRootDir()
{
    return Poco::Path(getDefaultCachePath() + CacheRootDir).toString();
}

std::string HttpTestUtil::getDefaultCacheTempDir()
{
    return Poco::Path(getDefaultCachePath() + CacheTempDir).toString();
}

std::string HttpTestUtil::getDefaultCacheDatabaseFile()
{
    return Poco::Path(getDefaultCachePath() + CacheDatabaseFile).toString();
}

std::string HttpTestUtil::getDefaultCertRootDir()
{
    return Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCertRootDir).toString();
}

std::string HttpTestUtil::getDefaultCertRootParentDir()
{
    return Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCertRootParentDir).toString();
}

Poco::Path HttpTestUtil::createDatabasePath(const std::string parentPath)
{
    Poco::Path databasePath(parentPath);
    Poco::Path cacheDir(CacheDir);
    databasePath.append(cacheDir);
    databasePath.setFileName(DatabaseFileName);
    return databasePath;
}

std::string HttpTestUtil::createCachedResponsedBodyFilePath(const std::string& cachePath,
        Request::HttpMethod httpMethod, const std::string& url)
{
    std::string key = HttpUtil::makeCacheKey(httpMethod, url);
    Poco::Path cacheRootDir(cachePath);
    Poco::Path cacheDir(CacheDir);
    cacheRootDir.append(cacheDir);
    return HttpUtil::makeCachedResponseBodyFilename(cacheRootDir, key);
}

std::string HttpTestUtil::makeUrl(const std::string& scheme, const std::string& host, unsigned short port,
            const std::string& path)
{
    return makeUrl(scheme, host, port, path, "");
}

std::string HttpTestUtil::makeUrl(const std::string& scheme, const std::string& host, unsigned short port,
            const std::string& path, const std::string& query)
{
    Poco::URI uri;
    uri.setScheme(scheme);
    uri.setHost(host);
    uri.setPort(port);
    uri.setPath(path);
    if (!query.empty()) {
        uri.setQuery(query);
    }
    return uri.toString();
}

size_t HttpTestUtil::readAllData(ResponseBodyStream::Ptr pResponseBodyStream, Poco::Buffer<char>& buffer)
{
    size_t totalBytes = 0;
    size_t capacity = buffer.capacity();
    while (!pResponseBodyStream->isEof() && totalBytes < capacity) {
        size_t bytes = pResponseBodyStream->read(buffer.begin() + totalBytes, capacity - totalBytes);
        if (bytes > 0) {
            totalBytes += bytes;
        }
    }
    return totalBytes;
}

void HttpTestUtil::loadDefaultCertData()
{
    std::string certRootParentDir = getDefaultCertRootParentDir();
    Poco::File file(certRootParentDir);
    file.createDirectories();
    Poco::File srcTestData(Poco::Path(FileUtil::convertToAbsolutePathString(
            EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForValidCert));
    srcTestData.copyTo(certRootParentDir);
}

void HttpTestUtil::setupHttpsServerDefaultSetting(HttpsTestServer& testServer)
{
    // set cert and start server
    std::string certRootDir = getDefaultCertRootDir();
    testServer.setCertUnitedFile(certRootDir + ServerCertFile);
    testServer.start(HttpTestConstants::DefaultHttpsPort);
}

std::string HttpTestUtil::getDefaultRootCaDirectory()
{
    return getDefaultCertRootDir() + ValidRootCaDir;
}

void HttpTestUtil::makeCacheDatabaseCorrupted()
{
    Poco::File databaseFile(HttpTestUtil::createDatabasePath(HttpTestUtil::getDefaultCachePath()));

    // replace start of database file with some garbage
    Poco::FileOutputStream fos(databaseFile.path());
    fos.seekp(0);
    fos << "Hello, world!";
    fos.close();
}

void HttpTestUtil::createInvalidCacheDatabase()
{
    FileUtil::createDirsIfAbsent(Poco::File(HttpTestUtil::getDefaultCacheRootDir()));

    Poco::File databaseFile(HttpTestUtil::createDatabasePath(HttpTestUtil::getDefaultCachePath()));
    // use a text file as a database file
    Poco::FileOutputStream fos(databaseFile.path());
    fos << "Hello, world!";
    fos.close();
}

HttpCacheMetadata::Ptr HttpTestUtil::createHttpCacheMetadata(const std::string& key, const std::string& url,
        size_t responseBodySize)
{
    HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
    pHttpCacheMetadata->setKey(key);
    pHttpCacheMetadata->setUrl(url);
    pHttpCacheMetadata->setHttpMethod(Test1HttpMethod);
    pHttpCacheMetadata->setStatusCode(Test1StatusCode);
    pHttpCacheMetadata->setStatusMessage(Test1StatusMessage);
    Headers::Ptr pHeaders = new Headers();
    pHeaders->set(Test1Header1, Test1HeaderValue1);
    pHttpCacheMetadata->setResponseHeaders(pHeaders);
    pHttpCacheMetadata->setResponseBodySize(responseBodySize);
    Poco::Timestamp timeStamp;
    HttpUtil::tryParseDate(Test1SentRequestTime, timeStamp);
    pHttpCacheMetadata->setSentRequestAtEpoch(timeStamp.epochTime());
    HttpUtil::tryParseDate(Test1ReceiveResponseTime, timeStamp);
    pHttpCacheMetadata->setReceivedResponseAtEpoch(timeStamp.epochTime());
    HttpUtil::tryParseDate(Test1CreatedMetadataTime, timeStamp);
    pHttpCacheMetadata->setCreatedAtEpoch(timeStamp.epochTime());
    return pHttpCacheMetadata;
}

std::string HttpTestUtil::createResponseTempFile(const std::string& tmpFilename, const std::string& responseBody,
        int no)
{
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    tempDir.createDirectories();
    std::string tempFilename = StringUtil::format("%s%s_%d", HttpTestUtil::getDefaultCacheTempDir().c_str(),
            tmpFilename.c_str(), no);
    Poco::FileOutputStream tempFileStream(tempFilename, std::ios::out | std::ios::trunc | std::ios::binary);
    tempFileStream.write(responseBody.c_str(), responseBody.size());
    tempFileStream.close();
    return tempFilename;
}

std::string HttpTestUtil::createResponseTempFileBySize(const std::string& tmpFilename, size_t responseBodySize)
{
    Poco::File tempDir(HttpTestUtil::getDefaultCacheTempDir());
    tempDir.createDirectories();
    std::string tempFilename = HttpTestUtil::getDefaultCacheTempDir() + tmpFilename;
    Poco::FileOutputStream tempFileStream(tempFilename, std::ios::out | std::ios::trunc | std::ios::binary);
    Poco::Buffer<char> buffer(responseBodySize);
    for (size_t i = 0; i < responseBodySize; i++) {
        buffer[i] = (i % 26) + 'a';
    }
    tempFileStream.write(buffer.begin(), responseBodySize);
    tempFileStream.close();
    return tempFilename;
}

} /* namespace test */
} /* namespace easyhttpcpp */
