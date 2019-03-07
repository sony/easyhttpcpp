/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include <fcntl.h>

#include "Poco/Path.h"
#include "Poco/File.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "TestFileUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::testutil::TestFileUtil;

namespace easyhttpcpp {
namespace Loader {
namespace test {

namespace {
static const std::string FileUtilTestRootDirPath = EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT);

static const std::string ParentDirForTest = "FileUtil";
static const std::string DirTobeCreate = "FileUtilTest";
static const std::string SubDirNoWritePermission = Poco::Path(ParentDirForTest, "TestNoWritePermission").toString();

// for createDirsIfAbsent
static const std::string DirAlreadyExists = "FileUtilTestDir";
static const Poco::Path SubDirectoryForTest(ParentDirForTest, "Test");
static const std::string DirAlreadyExistsInSubDirectory = Poco::Path(SubDirectoryForTest, "FileUtilTestDir").toString();
static const std::string FileAlreadyExists = "FileUtilTestFile";
static const std::string FileAlreadyExistsInSubDirectory
        = Poco::Path(SubDirectoryForTest, "FileUtilTestFile").toString();
static const std::string DirNotYetExists = "FileUtilTestDirNotYetExists";
static const Poco::Path DirTobeCreateWithSubDirectory(DirTobeCreate, "Test");
static const std::string DirNotYetExistsInSubDirectory =
        Poco::Path(DirTobeCreateWithSubDirectory, "FileUtilTestDirNotYetExists").toString();
static const std::string DirNoWritePermission = "FileUtilTestDirNoWritePermission";
static const std::string DirNoWritePermissionInSubDirectory =
        Poco::Path(SubDirNoWritePermission, "FileUtilTestDirNoWritePermission").toString();

// for removeDirsIfPresent
static const std::string DirHasSubDirectory = Poco::Path(SubDirectoryForTest, "FileUtilTestDir").toString();

// for removeFileIfPresent
static const std::string FileNotExists = "fileNotExists";
static const Poco::Path FileNotExistsWithSubDirectory("FileUtilNotExists", "Test");
static const std::string FileNotExistsInSubDirectory =
        Poco::Path(FileNotExistsWithSubDirectory, "fileNotExists").toString();
static const std::string FileNoWritePermission = "FileUtilTestFileNoWritePermission";
static const std::string FileNoWritePermissionInSubDirectory =
        Poco::Path(SubDirectoryForTest, "FileUtilTestFileNoWritePermission").toString();

// for moveFile
static const std::string SourceDir = "FileUtilSourceDir";
static const std::string SourceDirInSubDirectory = Poco::Path(SubDirectoryForTest, "fileUtilSourceDir").toString();
static const std::string SourceFileDepth0 = "FileUtilSourceFile";
static const std::string SourceFileDepth2 = Poco::Path(SubDirectoryForTest, "SourceFile").toString();
static const std::string DestFileDepth0 = "FileUtilDestFile";
static const std::string DestFileDepth2 = Poco::Path(SubDirectoryForTest, "DestFile").toString();
static const std::string SourceFileNotExists = "FileUtilSourceFileNotExists";

} /* namespace */

class FileUtilUnitTest : public testing::Test {
protected:

    void TearDown()
    {
        changePermission(&m_permissionChangeVector, true);
        removeFiles(&m_fileVector);
        removeDirectories(&m_dirVector);
    }

    void prepareDirectoriesAndFiles()
    {
        m_dirVector.push_back(DirAlreadyExists);
        m_dirVector.push_back(DirAlreadyExistsInSubDirectory);
        m_dirVector.push_back(DirNoWritePermission);
        m_dirVector.push_back(SubDirNoWritePermission);
        m_dirVector.push_back(SourceDir);
        m_dirVector.push_back(SourceDirInSubDirectory);

        prepareDirectories(&m_dirVector);

        m_fileVector.push_back(FileAlreadyExists);
        m_fileVector.push_back(FileAlreadyExistsInSubDirectory);
        m_fileVector.push_back(FileNoWritePermission);
        m_fileVector.push_back(FileNoWritePermissionInSubDirectory);
        m_fileVector.push_back(SourceFileDepth0);
        m_fileVector.push_back(SourceFileDepth2);

        prepareFiles(&m_fileVector);

        m_permissionChangeVector.push_back(DirNoWritePermission);
        m_permissionChangeVector.push_back(DirNoWritePermissionInSubDirectory);
        m_permissionChangeVector.push_back(SubDirNoWritePermission);
        m_permissionChangeVector.push_back(FileNoWritePermission);
        m_permissionChangeVector.push_back(FileNoWritePermissionInSubDirectory);

        changePermission(&m_permissionChangeVector, false);

        m_dirVector.push_back(ParentDirForTest);
        m_dirVector.push_back(DirTobeCreate);
        m_dirVector.push_back(DirNotYetExists);
        m_fileVector.push_back(DestFileDepth0);
    }

    Poco::Path createPath(const std::string path, bool isAbsolute)
    {
        if (path.empty()) {
            return Poco::Path();
        } else {
            if (isAbsolute) {
                return Poco::Path(FileUtilTestRootDirPath, path).absolute();
            } else {
                std::string testDirRelativePath;
                if (FileUtilTestRootDirPath[0] == '/') {
                    std::string currentPathString = Poco::Path::current();
                    testDirRelativePath = FileUtilTestRootDirPath.substr(currentPathString.size());
                } else {
                    testDirRelativePath = FileUtilTestRootDirPath;
                }
                return Poco::Path(testDirRelativePath, path);
            }
        }
    }

    void prepareDirectories(std::vector<std::string>* dirVector)
    {
        size_t vecSize = dirVector->size();
        for (size_t i = 0; i < vecSize; i++) {
            Poco::Path dirPath = createPath(dirVector->at(i), true);
            Poco::File(dirPath).createDirectories();
        }
    }

    void prepareFiles(std::vector<std::string>* fileVector)
    {
        size_t vecSize = fileVector->size();
        for (size_t i = 0; i < vecSize; i++) {
            Poco::Path filePath = createPath(fileVector->at(i), true);
            bool result = Poco::File(filePath).createFile();
            if (!result) {
                // fallback, if Poco::File::createFile fails
                FILE* fp = fopen(filePath.toString().c_str(), "w+");
                if (fp) {
                    fclose(fp);
                }
            }
        }
    }

    void removeDirectories(std::vector<std::string>* dirVector)
    {
        size_t vecSize = dirVector->size();
        for (size_t i = 0; i < vecSize; i++) {
            Poco::Path dirPath = createPath(dirVector->at(i), true);
            if (Poco::File(dirPath).exists()) {
                Poco::File(dirPath).remove(true);
            }
        }
    }

    void removeFiles(std::vector<std::string>* fileVector)
    {
        size_t vecSize = fileVector->size();
        for (size_t i = 0; i < vecSize; i++) {
            Poco::Path filePath = createPath(fileVector->at(i), true);
            if (Poco::File(filePath).exists()) {
                Poco::File(filePath).remove(true);
            }
        }
    }

    void changePermission(std::vector<std::string>* targetVector, bool enableReadWrite)
    {
        size_t vecSize = targetVector->size();
        for (size_t i = 0; i < vecSize; i++) {
            Poco::Path path = createPath(targetVector->at(i), true);
            if (enableReadWrite) {
                // change to rwx
                TestFileUtil::changeAccessPermission(path, EASYHTTPCPP_FILE_PERMISSION_FULL_ACCESS);
            } else {
                //change to --x
                TestFileUtil::changeAccessPermission(path, EASYHTTPCPP_FILE_PERMISSION_ALLUSER_EXECUTE_ONLY);
            }
        }
    }

public:
    std::vector<std::string> m_dirVector;
    std::vector<std::string> m_fileVector;
    std::vector<std::string> m_permissionChangeVector;
};

class CreateDirsIfAbsentTestParam {
public:
    const std::string m_dir;
    bool m_expectedResult;
    bool m_isAbsolute;
};

static const CreateDirsIfAbsentTestParam CreateDirsIfAbsentTestParams[] = {
    // m_dir    m_expectedResult    m_isAbsolute
    {DirAlreadyExists, true, true},
    {DirAlreadyExists, true, false},
    {DirAlreadyExistsInSubDirectory, true, true},
    {DirAlreadyExistsInSubDirectory, true, false},
    {FileAlreadyExists, true, true},
    {FileAlreadyExists, true, false},
    {FileAlreadyExistsInSubDirectory, true, true},
    {FileAlreadyExistsInSubDirectory, true, false},
    {DirNotYetExists, true, true},
    {DirNotYetExists, true, false},
    {DirNotYetExistsInSubDirectory, true, true},
    {DirNotYetExistsInSubDirectory, true, false},
    {"", false, false},
};

class FileUtilCreateDirsIfAbsentParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<CreateDirsIfAbsentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilCreateDirsIfAbsentParameterizedTest,
        ::testing::ValuesIn(CreateDirsIfAbsentTestParams));

TEST_P(FileUtilCreateDirsIfAbsentParameterizedTest, createDirsIfAbsent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::createDirsIfAbsent()
    // Then: FileUtil::createDirsIfAbsent() returns the appropriate results
    Poco::File fileToBeCreated(createPath(GetParam().m_dir, GetParam().m_isAbsolute));
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::createDirsIfAbsent(fileToBeCreated));
}

static const CreateDirsIfAbsentTestParam CreateDirsIfAbsentWhenDirNoWritePermissionTestParams[] = {
    // m_dir    m_expectedResult    m_isAbsolute
    {DirNoWritePermission, true, true},
    {DirNoWritePermission, true, false},
    {DirNoWritePermissionInSubDirectory, false, true},
    {DirNoWritePermissionInSubDirectory, false, false},
};

class FileUtilCreateDirsIfAbsentWhenDirNoWritePermissionParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<CreateDirsIfAbsentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilCreateDirsIfAbsentWhenDirNoWritePermissionParameterizedTest,
        ::testing::ValuesIn(CreateDirsIfAbsentWhenDirNoWritePermissionTestParams));

TEST_P(FileUtilCreateDirsIfAbsentWhenDirNoWritePermissionParameterizedTest,
        createDirsIfAbsent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::createDirsIfAbsent()
    // Then: FileUtil::createDirsIfAbsent() returns the appropriate results
    Poco::File fileToBeCreated(createPath(GetParam().m_dir, GetParam().m_isAbsolute));
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::createDirsIfAbsent(fileToBeCreated));
}

class RemoveDirsIfPresentTestParam {
public:
    const std::string m_path;
    bool m_expectedResult;
    bool m_isAbsolute;
};

static const RemoveDirsIfPresentTestParam RemoveDirsIfPresentTestParams[] = {
    // m_path    m_expectedResult    m_isAbsolute
    {DirNotYetExists, true, true},
    {DirNotYetExists, true, false},
    {DirNotYetExistsInSubDirectory, true, true},
    {DirNotYetExistsInSubDirectory, true, false},
    {DirAlreadyExists, true, true},
    {DirAlreadyExists, true, false},
    {DirAlreadyExistsInSubDirectory, true, true},
    {DirAlreadyExistsInSubDirectory, true, false},
    {DirHasSubDirectory, true, true},
    {DirHasSubDirectory, true, false},
    {"", false, false},
};

class FileUtilRemoveDirsIfPresentParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<RemoveDirsIfPresentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilRemoveDirsIfPresentParameterizedTest,
        ::testing::ValuesIn(RemoveDirsIfPresentTestParams));

TEST_P(FileUtilRemoveDirsIfPresentParameterizedTest, removeDirsIfPresent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    m_dirVector.push_back(DirNoWritePermissionInSubDirectory);
    prepareDirectoriesAndFiles();

    // When: call FileUtil::removeDirsIfPresent()
    // Then: FileUtil::removeDirsIfPresent() returns the appropriate results
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::removeDirsIfPresent(createPath(GetParam().m_path,
            GetParam().m_isAbsolute)));
}

static const RemoveDirsIfPresentTestParam RemoveDirsIfPresentWhenDirNoWritePermissionTestParams[] = {
    // m_path    m_expectedResult    m_isAbsolute
    {DirNoWritePermission, false, true},
    {DirNoWritePermission, false, false},
    {DirNoWritePermissionInSubDirectory, false, true},
    {DirNoWritePermissionInSubDirectory, false, false},
};

class FileUtilRemoveDirsIfPresentWhenDirNoWritePermissionParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<RemoveDirsIfPresentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilRemoveDirsIfPresentWhenDirNoWritePermissionParameterizedTest,
        ::testing::ValuesIn(RemoveDirsIfPresentWhenDirNoWritePermissionTestParams));

TEST_P(FileUtilRemoveDirsIfPresentWhenDirNoWritePermissionParameterizedTest,
        removeDirsIfPresent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    m_dirVector.push_back(DirNoWritePermissionInSubDirectory);
    prepareDirectoriesAndFiles();

    // When: call FileUtil::removeDirsIfPresent()
    // Then: FileUtil::removeDirsIfPresent() returns the appropriate results
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::removeDirsIfPresent(createPath(GetParam().m_path,
            GetParam().m_isAbsolute)));
}

class RemoveFileIfPresentTestParam {
public:
    const std::string m_file;
    bool m_expectedResult;
    bool m_isAbsolute;
};

static const RemoveFileIfPresentTestParam RemoveFileIfPresentTestParams[] = {
    // m_file    m_expectedResult    m_isAbsolute
    {FileNotExists, true, true},
    {FileNotExists, true, false},
    {FileNotExistsInSubDirectory, true, true},
    {FileNotExistsInSubDirectory, true, false},
    {FileAlreadyExists, true, true},
    {FileAlreadyExists, true, false},
    {FileAlreadyExistsInSubDirectory, true, true},
    {FileAlreadyExistsInSubDirectory, true, false},
};

class FileUtilRemoveFileIfPresentParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<RemoveFileIfPresentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilRemoveFileIfPresentParameterizedTest,
        ::testing::ValuesIn(RemoveFileIfPresentTestParams));

TEST_P(FileUtilRemoveFileIfPresentParameterizedTest, removeFileIfPresent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::removeDirsIfPresent()
    // Then: FileUtil::removeDirsIfPresent() returns the appropriate results
    Poco::File fileToBeRemoved(createPath(GetParam().m_file, GetParam().m_isAbsolute));
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::removeFileIfPresent(fileToBeRemoved));
}

static const RemoveFileIfPresentTestParam RemoveFileIfPresentWhenDirNoWritePermissionTestParams[] = {
    // m_file    m_expectedResult    m_isAbsolute
    {FileNoWritePermission, true, true},
    {FileNoWritePermission, true, false},
    {FileNoWritePermissionInSubDirectory, true, true},
    {FileNoWritePermissionInSubDirectory, true, false},
};

class FileUtilRemoveFileIfPresentWhenDirNoWritePermissionParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<RemoveFileIfPresentTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilRemoveFileIfPresentWhenDirNoWritePermissionParameterizedTest,
        ::testing::ValuesIn(RemoveFileIfPresentWhenDirNoWritePermissionTestParams));

TEST_P(FileUtilRemoveFileIfPresentWhenDirNoWritePermissionParameterizedTest,
        removeFileIfPresent_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::removeDirsIfPresent()
    // Then: FileUtil::removeDirsIfPresent() returns the appropriate results
    Poco::File fileToBeRemoved(createPath(GetParam().m_file, GetParam().m_isAbsolute));
    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::removeFileIfPresent(fileToBeRemoved));
}

class MoveFileTestParam {
public:
    const std::string m_source;
    const std::string m_destination;
    bool m_expectedResult;
};

static const MoveFileTestParam MoveFileTestParams[] = {
    // m_source    m_destination    m_expectedResult
    {SourceDir, DestFileDepth0, false},
    {SourceDirInSubDirectory, DestFileDepth0, false},
    {SourceFileDepth0, FileAlreadyExists, true},
    {SourceFileDepth0, FileAlreadyExistsInSubDirectory, true},
    {SourceFileDepth0, DirAlreadyExists, false},
    {SourceFileDepth0, DirAlreadyExistsInSubDirectory, false},
    {SourceFileDepth0, DestFileDepth0, true},
    {SourceFileDepth0, DestFileDepth2, true},
    {SourceFileDepth2, DestFileDepth0, true},
    {SourceFileDepth2, DestFileDepth2, true},
    {SourceFileDepth0, SourceFileDepth0, true},
    {SourceFileNotExists, DestFileDepth0, false},
};

class FileUtilMoveFileParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<MoveFileTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilMoveFileParameterizedTest, ::testing::ValuesIn(MoveFileTestParams));

TEST_P(FileUtilMoveFileParameterizedTest, moveFile_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::moveFile()
    // Then: FileUtil::moveFile() returns the appropriate results
    Poco::File source(createPath(GetParam().m_source, true));
    Poco::File destination(createPath(GetParam().m_destination, true));

    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::moveFile(source, destination));
}

static const MoveFileTestParam MoveFileWhenDirNoWritePermissionTestParams[] = {
    // m_source    m_destination    m_expectedResult
    {SourceFileDepth0, FileNoWritePermission, true},
    {SourceFileDepth0, FileNoWritePermissionInSubDirectory, true},
    {SourceFileDepth0, DirNoWritePermission, false},
    {SourceFileDepth0, DirNoWritePermissionInSubDirectory, false},
};

class FileUtilMoveFileWhenDirNoWritePermissionParameterizedTest : public FileUtilUnitTest,
public ::testing::WithParamInterface<MoveFileTestParam> {
};

INSTANTIATE_TEST_CASE_P(FileUtilUnitTest, FileUtilMoveFileWhenDirNoWritePermissionParameterizedTest,
        ::testing::ValuesIn(MoveFileWhenDirNoWritePermissionTestParams));

TEST_P(FileUtilMoveFileWhenDirNoWritePermissionParameterizedTest,
        moveFile_BehavesInAccordanceWithTheParamter)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // When: call FileUtil::moveFile()
    // Then: FileUtil::moveFile() returns the appropriate results
    Poco::File source(createPath(GetParam().m_source, true));
    Poco::File destination(createPath(GetParam().m_destination, true));

    EXPECT_EQ(GetParam().m_expectedResult, FileUtil::moveFile(source, destination));
}

TEST_F(FileUtilUnitTest,
        moveFile_ReturnsTrue_WhenSourceAndDestinationAreSameFileButOneIsTheAbsolutePathTheOtherIsTheRelativePath)
{
    // Given: prepare directories and files
    prepareDirectoriesAndFiles();

    // create Poco::File object, source is absolute path, and destination is relative path
    Poco::File source(createPath(SourceFileDepth0, true));
    Poco::File destination(createPath(SourceFileDepth0, false));

    // When: call FileUtil::moveFile()
    // Then: FileUtil::moveFile() returns true
    EXPECT_TRUE(FileUtil::moveFile(source, destination));
}

} /* namespace test */
} /* namespace Loader */
} /* namespace easyhttpcpp */
