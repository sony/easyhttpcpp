/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/CommonMacros.h"
#include "easyhttpcpp/common/ProjectVersion.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {
namespace test {

TEST(ProjectVersionUnitTest, getMajor_ReturnsMajorVersion)
{
    // Given: -

    // When: Call ProjectVersion::getMajor()
    // Then: getMajor returns major version as string
    EXPECT_EQ(EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MAJOR), ProjectVersion::getMajor());
}

TEST(ProjectVersionUnitTest, getMinor_ReturnsMinorVersion)
{
    // Given: -

    // When: Call ProjectVersion::getMinor()
    // Then: getMinor returns minor version as string
    EXPECT_EQ(EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MINOR), ProjectVersion::getMinor());
}

TEST(ProjectVersionUnitTest, getPatch_ReturnsPatchVersion)
{
    // Given: -

    // When: Call ProjectVersion::getPatch()
    // Then: getPatch returns patch version as string
    EXPECT_EQ(EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_PATCH), ProjectVersion::getPatch());
}

TEST(ProjectVersionUnitTest, getExtension_ReturnsExtensionString)
{
    // Given: -

    // When: Call ProjectVersion::getExtension()
    // Then: getExtension returns extension as string
    EXPECT_EQ(EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_EXT), ProjectVersion::getExtension());
}

TEST(ProjectVersionUnitTest, asString_ReturnsFormattedVersionString)
{
    // Given: -

    // When: Call ProjectVersion::asString()
    // Then: asString returns formatted version as string
    std::string versionStr = StringUtil::format("%s.%s.%s", EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MAJOR),
            EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_MINOR), EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_PATCH));
    std::string extension = EASYHTTPCPP_STRINGIFY_MACRO(PACKAGE_VERSION_EXT);
    if (!extension.empty()) {
        versionStr.append(StringUtil::format("-%s", extension.c_str()));
    }
    EXPECT_EQ(versionStr, ProjectVersion::asString());
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
