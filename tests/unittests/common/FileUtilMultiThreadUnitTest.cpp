/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/ThreadPool.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/FileUtil.h"

namespace easyhttpcpp {
namespace common {
namespace test {
namespace {
const Poco::Path TestRootPath(EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT), "FileUtilMultiThreadUnitTest");
// Windowsでは260byteの制限を解除する為に,ファイル名にディレクトリセパレータを含むパスを扱う場合は,
// Path(const Path & parent, const Path & relative); のinterfaceを使う必要があります.
const Poco::Path TestTargetDir(TestRootPath, Poco::Path("a/b/c/d/e/f"));

class DirsCreateRunner : public Poco::Runnable {
public:
    bool m_success;

    DirsCreateRunner() : m_success(false)
    {
    }

    virtual void run()
    {
        m_success = FileUtil::createDirsIfAbsent(TestTargetDir);
    }
};
} /* namespace */

class FileUtilMultiThreadUnitTest : public testing::Test {
public:

    virtual void TearDown()
    {
        FileUtil::removeDirsIfPresent(TestRootPath);
    }
};

// test for bug in Poco::File::createDirectories()
// see, https://github.com/pocoproject/poco/issues/1573
TEST_F(FileUtilMultiThreadUnitTest, createDirsIfAbsent_Succeeds_WhenCalledOnMultipleThreadsWithSameDir)
{
    // Given: set up thread pool
    Poco::ThreadPool threadPool;
    const int threadTotal = 5;

    // When: call createDirsIfAbsent() on multiple threads
    DirsCreateRunner runners[threadTotal];
    for (int runnerCount = 0; runnerCount < threadTotal; ++runnerCount) {
        threadPool.start(runners[runnerCount]);
    }
    threadPool.joinAll();

    // Then: directory is created successfully
    for (int runnerCount = 0; runnerCount < threadTotal; ++runnerCount) {
        EXPECT_TRUE(runners[runnerCount].m_success) << "runner index: " << runnerCount;
    }
    EXPECT_TRUE(Poco::File(TestTargetDir.absolute()).exists());
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
