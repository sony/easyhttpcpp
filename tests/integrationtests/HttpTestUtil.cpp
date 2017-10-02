/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/URI.h"

#include "easyhttpcpp/common/CommonMacros.h"

#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

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

std::string HttpTestUtil::getDefaultCacheParentPath()
{
    return std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCacheParentPath;
}

std::string HttpTestUtil::getDefaultCachePath()
{
    return std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCachePath;
}

std::string HttpTestUtil::getDefaultCacheRootDir()
{
    return getDefaultCachePath() + CacheRootDir;
}

std::string HttpTestUtil::getDefaultCacheTempDir()
{
    return getDefaultCachePath() + CacheTempDir;
}

std::string HttpTestUtil::getDefaultCacheDatabaseFile()
{
    return getDefaultCachePath() + CacheDatabaseFile;
}

std::string HttpTestUtil::getDefaultCertRootDir()
{
    return std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCertRootDir;
}

std::string HttpTestUtil::getDefaultCertRootParentDir()
{
    return std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + DefaultCertRootParentDir;
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

std::string HttpTestUtil::makeUrl(const std::string& scheme, const std::string& host, unsigned int port,
            const std::string& path)
{
    return makeUrl(scheme, host, port, path, "");
}

std::string HttpTestUtil::makeUrl(const std::string& scheme, const std::string& host, unsigned int port,
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
    Poco::File srcTestData(std::string(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT)) + TestDataForValidCert);
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

} /* namespace test */
} /* namespace easyhttpcpp */
