/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"
#include "TestFileUtil.h"

#include "HttpCacheInternal.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::TestFileUtil;

namespace easyhttpcpp {
namespace test {

static const char* const DefaultCachePath = "/HttpCache/";
static const size_t DefaultCacheMaxSize = 100;
static const char* const DefaultCacheTempDirectory = "/HttpCache/cache/temp/";

class HttpCacheInternalUnitTest : public testing::Test {
protected:

    void SetUp()
    {
        // Code here will be called immediately after the constructor (right before each test).
        Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
        FileUtil::removeDirsIfPresent(path);
    }
};

// path, maxSize 指定
TEST_F(HttpCacheInternalUnitTest, constructor_setsPathAndMaxSize_WhenSpecifiedParameter)
{
    // Given: specified path
    Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
    size_t maxSize = 100;

    // When: create HttpCacheInternal
    HttpCacheInternal httpCache(path, maxSize);

    // Then: getPath get specified absolute path and default value
    const Poco::Path& gottenPath = httpCache.getPath();
    EXPECT_EQ(path.toString(), gottenPath.toString());
    EXPECT_EQ(maxSize, httpCache.getMaxSize());
    EXPECT_EQ(0, httpCache.getSize());
    Poco::Path expectTempDir(
            StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCacheTempDirectory));
    Poco::Path actualTempDir(httpCache.getTempDirectory());
    EXPECT_EQ(expectTempDir.absolute().toString(), actualTempDir.toString());
    easyhttpcpp::common::CacheManager::Ptr pCacheManager = httpCache.getCacheManager();
    ASSERT_FALSE(pCacheManager.isNull());
}

// tempDirectory がまだ存在していない場合。
// tempDirectory が取得できる。
// tempDirectory が作成される。
TEST_F(HttpCacheInternalUnitTest, getTempDirectory_ReturnsTempDirectoryAndCreatesTempDirectory_WhenNotExistTempDirectory)
{
    // Given: create HttpCacheInternal
    Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
    HttpCacheInternal httpCache(path, DefaultCacheMaxSize);
    Poco::Path actualTmpPath(
            StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCacheTempDirectory));
    Poco::File actualTempDir(actualTmpPath.absolute());
    ASSERT_FALSE(actualTempDir.exists());

    // When: call tempDirectory
    const std::string tempDirStr = httpCache.getTempDirectory();

    // Then: return temp directory of absolute path and create temp directpry
    Poco::Path gottenTmpPath(tempDirStr);
    EXPECT_EQ(actualTmpPath.absolute().toString(), gottenTmpPath.toString());
    Poco::File gottenTempDir(gottenTmpPath);
    EXPECT_TRUE(gottenTempDir.exists());
}

// tempDirectory がまだ存在している場合。
// tempDirectory が取得できる。
// tempDirectory はそのまま。
// すでに存在しているファイルが消えたりしない。
TEST_F(HttpCacheInternalUnitTest, getTempDirectory_ReturnsTempDirectoryAndNoEffectTempDirectory_WhenExistTempDirectory)
{
    // Given: create temp directory, and test file
    Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
    HttpCacheInternal httpCache(path, DefaultCacheMaxSize);
    Poco::Path actualTmpPath(
            StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCacheTempDirectory));
    Poco::File actualTempDir(actualTmpPath.absolute());
    actualTempDir.createDirectories();
    ASSERT_TRUE(actualTempDir.exists());
    Poco::File testFile(Poco::Path(StringUtil::format(
            "%s%stest.dat", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCacheTempDirectory)).absolute());
    ASSERT_TRUE(testFile.createFile());

    // When: call tempDirectory
    const std::string tempDirStr = httpCache.getTempDirectory();

    // Then: return temp directory of absolute path and no effect temp directory
    Poco::Path gottenTmpPath(tempDirStr);
    EXPECT_EQ(actualTmpPath.absolute().toString(), gottenTmpPath.toString());
    Poco::File gottenTempDir(gottenTmpPath);
    EXPECT_TRUE(gottenTempDir.exists());
    EXPECT_TRUE(testFile.exists());
}

// temp directory が作成できない場合(parent となる cache ディレクトリをread only する。)
// ExecutionException が throw される。
TEST_F(HttpCacheInternalUnitTest, getTempDirectory_ThrowsException_WhenIOErrorOccurred)
{
    // Given: set read only parent of temp directory.
    Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
    HttpCacheInternal httpCache(path, DefaultCacheMaxSize);
    Poco::Path actualTmpPath(
            StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCacheTempDirectory));
    Poco::Path parentPath = actualTmpPath.parent();
    Poco::File parentDir(parentPath.absolute());
    parentDir.createDirectories();
    TestFileUtil::changeAccessPermission(parentPath, EASYHTTPCPP_FILE_PERMISSION_ALLUSER_READ_ONLY);

    // When: call tempDirectory
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(httpCache.getTempDirectory(), HttpExecutionException, 100702);
    TestFileUtil::changeAccessPermission(parentPath, EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS);
}

// getCacheManager
TEST_F(HttpCacheInternalUnitTest, getCacheManager_ReturnsCacheManager)
{
    // Given: create HttpCacheInternal
    Poco::Path path(StringUtil::format("%s%s", EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), DefaultCachePath));
    HttpCacheInternal httpCache(path, DefaultCacheMaxSize);

    // When: call getCacheManager
    // Then: return CacheManager
    EXPECT_FALSE(httpCache.getCacheManager().isNull());
}

} /* namespace test */
} /* namespace easyhttpcpp */
