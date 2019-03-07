/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "gtest/gtest.h"

#include "easyhttpcpp/common/StringUtil.h"

#if defined(_WIN64)
#define LONG_LONG_MIN _I64_MIN
#elif defined(_WIN32)
#define LONG_LONG_MIN LONG_MIN
#endif

namespace easyhttpcpp {
namespace common {
namespace test {

class StringUtilTest : public testing::Test {
protected:

    void SetUp()
    {
    }

    void TearDown()
    {
    }

};

// %c

TEST_F(StringUtilTest, format_Succeeds_WhenFormatChar)
{
    const char* format = "%c%c%c";
    char str1 = 'a';
    char str2 = 'b';
    char str3 = 'c';

    std::string formatMessage = StringUtil::format(format, str1, str2, str3);

    ASSERT_STREQ("abc", formatMessage.c_str());
}

// %s with char array

TEST_F(StringUtilTest, format_Succeeds_WhenFormatWithCharArray)
{
    const char* format = "%s";
    char message[] = "hoge";

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ(message, formatMessage.c_str());
}

// %s with std::string.c_str())

TEST_F(StringUtilTest, format_Succeeds_WhenFormatWithCharArrayOfStdStringCharArray)
{
    const char* format = "%s";
    std::string message = "hoge";

    std::string formatMessage = StringUtil::format(format, message.c_str());

    ASSERT_STREQ(message.c_str(), formatMessage.c_str());
}

// %d with int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatInt)
{
    const char* format = "%d";
    int message = 1;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1", formatMessage.c_str());
}

// %d with short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatShort)
{
    const char* format = "%d";
    short message = 1;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1", formatMessage.c_str());
}

// %u with unsigned int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedInt)
{
    const char* format = "%u";
    unsigned int message = 1;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1", formatMessage.c_str());
}

// %u with unsigned short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedShort)
{
    const char* format = "%u";
    unsigned short message = 1;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1", formatMessage.c_str());
}

// %o with int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatIntOutputOctal)
{
    const char* format = "%o";
    int message = 8;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("10", formatMessage.c_str());
}

// %o with short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatShortOutputOctal)
{
    const char* format = "%o";
    short message = 8;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("10", formatMessage.c_str());
}

// %o with unsigned int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedIntOutputOctal)
{
    const char* format = "%o";
    unsigned int message = 8;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("10", formatMessage.c_str());
}

// %o with unsigned short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedShortOutputOctal)
{
    const char* format = "%o";
    unsigned short message = 8;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("10", formatMessage.c_str());
}

// %x with int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatIntOutputHex)
{
    const char* format = "%x";
    int message = 15;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("f", formatMessage.c_str());
}

// %x with short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatShortOutputHex)
{
    const char* format = "%x";
    short message = 15;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("f", formatMessage.c_str());
}

// %x with unsigned int

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedIntOutputHex)
{
    const char* format = "%x";
    unsigned int message = 15;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("f", formatMessage.c_str());
}

// %x with unsigned short

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedShortOutputHex)
{
    const char* format = "%x";
    unsigned short message = 15;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("f", formatMessage.c_str());
}

// %f with float

TEST_F(StringUtilTest, format_Succeeds_WhenFormatFloatOutputRealNumber)
{
    const char* format = "%f";
    float message = 1.5;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1.500000", formatMessage.c_str());
}

// %e with float

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedShortOutputRealNumberWithIndexNumber)
{
    const char* format = "%e";
    float message = 1.5;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1.500000e+00", formatMessage.c_str());
}

// %g with float

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedShortOutputRealNumberOfOptimalForm)
{
    const char* format = "%g";
    float message = 1.5;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("1.5", formatMessage.c_str());
}

// TODO: 32bit/64bit tests
#if 0
// %ld with long

TEST_F(StringUtilTest, format_Succeeds_WhenFormatLong)
{
    std::string format = "%ld";
    long message = LONG_MIN;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("-2147483648", formatMessage.c_str());
}

// %lu with unsigned long

TEST_F(StringUtilTest, format_Succeeds_WhenFormatUnsignedLong)
{
    std::string format = "%lu";
    unsigned long message = LONG_MIN;

    std::string formatMessage = StringUtil::format(format, message);

    ASSERT_STREQ("2147483648", formatMessage.c_str());
}
#endif

// %lld %llu with long long

TEST_F(StringUtilTest, format_Succeeds_WhenFormatLongLong)
{
    const char* format1 = "%lld";
    const char* format2 = "%llu";
    long long message1 = -12345LL;
    unsigned long long message2 = 12345ULL;

    std::string formatMessage1 = StringUtil::format(format1, message1);
    std::string formatMessage2 = StringUtil::format(format2, message2);

    ASSERT_STREQ("-12345", formatMessage1.c_str());
    ASSERT_STREQ("12345", formatMessage2.c_str());
}

class IsNullOrEmptyTestParam {
public:
    const std::string* m_pInput;
    bool m_expected;
};

static const IsNullOrEmptyTestParam IsNullOrEmptyTestParams[] = {
    // input, expected
    {NULL, true},
    {new std::string(""), true},
    {new std::string(" "), false},
    {new std::string("test string"), false},
};

class StringUtilIsNullOrEmptyParameterizedTest : public ::testing::TestWithParam<IsNullOrEmptyTestParam> {
};
INSTANTIATE_TEST_CASE_P(StringUtilTest, StringUtilIsNullOrEmptyParameterizedTest,
        ::testing::ValuesIn(IsNullOrEmptyTestParams));

TEST_P(StringUtilIsNullOrEmptyParameterizedTest, isNullOrEmpty_ReturnsValidResult)
{
    // Given: This test has no no specific conditions

    // When: call StringUtil::isNullOrEmpty
    // Then: isNullOrEmpty returns valid result
    EXPECT_EQ(GetParam().m_expected, StringUtil::isNullOrEmpty(GetParam().m_pInput));
}

class FormatVersionTestParam {
public:
    const std::string m_major;
    const std::string m_minor;
    const std::string m_patch;
    const std::string m_extension;
    const std::string m_expectedVersion;
};

static const FormatVersionTestParam FormatVersionTestParams[] = {
    // major, minor, patch, extension, expected
    {"1", "2", "3", "", "1.2.3"},
    {"0", "0", "0", "", "0.0.0"},
    {"1", "", "", "", "1.."},
    {"", "1", "", "", ".1."},
    {"", "", "1", "", "..1"},
    {"1", " ", " ", "", "1. . "},
    {" ", "1", " ", "", " .1. "},
    {" ", " ", "1", "", " . .1"},
    {" 1 ", "2", "3", "", " 1 .2.3"},
    {"1", "2", "3", "testExtension", "1.2.3-testExtension"},
    {"1", "2", "3", "4", "1.2.3-4"},
    {"1", "2", "3", "testExtension-4", "1.2.3-testExtension-4"},
    {"1", "2", "3", "test Extension", "1.2.3-test Extension"},
    {"1", "2", "3", " test Extension ", "1.2.3- test Extension "},
    {"1", "2", "3", " ", "1.2.3- "},
    {"", "", "", "testExtension", "..-testExtension"},
};

class StringUtilFormatVersionParameterizedTest : public ::testing::TestWithParam<FormatVersionTestParam> {
};
INSTANTIATE_TEST_CASE_P(StringUtilTest, StringUtilFormatVersionParameterizedTest,
        ::testing::ValuesIn(FormatVersionTestParams));

TEST_P(StringUtilFormatVersionParameterizedTest, formatVersion_ReturnsFormattedVersionString)
{
    // Given: This test has no no specific conditions

    // When: call StringUtil::formatVersion
    // Then: formatVersion returns formatted version as string
    EXPECT_EQ(GetParam().m_expectedVersion, StringUtil::formatVersion(GetParam().m_major, GetParam().m_minor,
            GetParam().m_patch, GetParam().m_extension));
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
