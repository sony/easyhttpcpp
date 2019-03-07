/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/NumberFormatter.h"

#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/common/StringUtil.h"

#if defined(_WIN64)
#define ULONG_LONG_MAX _UI64_MAX
#define LONG_LONG_MIN _I64_MIN
#define LONG_LONG_MAX _I64_MAX
#elif defined(_WIN32)
#define ULONG_LONG_MAX ULONG_MAX
#define LONG_LONG_MIN LONG_MIN
#define LONG_LONG_MAX LONG_MAX
#endif

namespace easyhttpcpp {
namespace db {
namespace test {

class ContentValuesUnitTest : public testing::Test {
protected:

    ContentValuesUnitTest()
    {
    }

    virtual ~ContentValuesUnitTest()
    {
    }
};

class ContentValuesTestParam {
public:
    std::string key;
    int intValue;
    unsigned int uintValue;
    float floatValue;
    short shortValue;
    double doubleValue;
    long longValue;
    unsigned long ulongValue;
    long long longlongValue;
    unsigned long long ulonglongValue;
    std::string stringValue;
};

static const ContentValuesTestParam ContentValuesTestParams[] = {
    /* key, int, unsigned int, float, short, double, long, unsigned long, long long, unsigned long long, string*/
    {"key1", INT_MIN, 0, FLT_MIN, SHRT_MIN, DBL_MIN, LONG_MIN, 0, LONG_LONG_MIN, 0, "value1"},
    {"key2", -100, 100, -100.234f, -200, -1.23456, -100000, 2000, -90000000000, 800, " "},
    {"key3", 0, 500, 0.00f, 0, 0.00, 0, 30000, 0, 15000, ""},
    {"key4", 100, 600, 100.234f, 200, 200.456789, 3000000, 400000000, 90000000000, 9000000, "test string value "},
    {"key5", INT_MAX, UINT_MAX, FLT_MAX, SHRT_MAX, DBL_MAX, LONG_MAX, ULONG_MAX, LONG_LONG_MAX, ULONG_LONG_MAX, "test"},
};

class ContentValuesParameterizedTest : public ::testing::TestWithParam<ContentValuesTestParam> {
};
INSTANTIATE_TEST_CASE_P(ContentValuesUnitTest, ContentValuesParameterizedTest,
        ::testing::ValuesIn(ContentValuesTestParams));

TEST_P(ContentValuesParameterizedTest, putIntAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put int value
    contentValues.put(GetParam().key, GetParam().intValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().intValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putUnsignedIntAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put unsigned int value
    contentValues.put(GetParam().key, GetParam().uintValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().uintValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putFloatAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put float value
    contentValues.put(GetParam().key, GetParam().floatValue);

    // Then: getStringValue returns put value as string
    EXPECT_STREQ(Poco::NumberFormatter::format(GetParam().floatValue).c_str(),
            contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putShortAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put short value
    contentValues.put(GetParam().key, GetParam().shortValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().shortValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putDoubleAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().doubleValue);

    // Then: getStringValue returns put value as string
    EXPECT_STREQ(Poco::NumberFormatter::format(GetParam().doubleValue).c_str(),
            contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putLongAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().longValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().longValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putUnsignedLongAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().ulongValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().ulongValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putLongLongAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().longlongValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().longlongValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putUnsignedLongLongAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().ulonglongValue);

    // Then: getStringValue returns put value as string
    std::stringstream ss;
    ss << GetParam().ulonglongValue;
    EXPECT_STREQ(ss.str().c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_P(ContentValuesParameterizedTest, putStringAndGetSringValue_Succeeds)
{
    // Given: create ContentValues object
    ContentValues contentValues;

    // When: put double value
    contentValues.put(GetParam().key, GetParam().stringValue);

    // Then: getStringValue returns put value as string
    EXPECT_STREQ(GetParam().stringValue.c_str(), contentValues.getStringValue(GetParam().key).c_str());
}

TEST_F(ContentValuesUnitTest, put_OverwriteFormerValue_WhenCallWithSameKey)
{
    // Given: call put use "key_test" as key
    ContentValues contentValues;

    contentValues.put("key_test", 100);
    ASSERT_STREQ("100", contentValues.getStringValue("key_test").c_str()) << "put string value failed";

    // When: call put again use "key_test" as key
    contentValues.put("key_test", 2000);

    // Then: key_test should be overwritten
    // number of key stored in ContentValues object should be 1.
    EXPECT_STREQ("2000", contentValues.getStringValue("key_test").c_str());

    std::vector<std::string> keys;
    contentValues.getKeys(keys);
    EXPECT_EQ(1, keys.size());
}

TEST_F(ContentValuesUnitTest, put_Succeeds_WhenCallWithEmptyKey)
{
    // When: call put use "" as key
    ContentValues contentValues;
    contentValues.put("", 100);

    // Then: put succeeds, getStringValue retuns put value
    EXPECT_STREQ("100", contentValues.getStringValue("").c_str());
}

TEST_F(ContentValuesUnitTest, getStringValue_ThrowsException_WhenNotFoundKey)
{
    // Given: put the key "abc" and "def"
    ContentValues contentValues;

    contentValues.put("abc", "test A");
    contentValues.put("def", "test B");

    try {
        // When: call getStringValue with the key that ContentValues
        // object does not have.
        contentValues.getStringValue("ghi");
    } catch (const SqlIllegalArgumentException& e) {
        // Then: getStringValue() throws SqlIllegalArgumentException
        EXPECT_EQ(0, e.getExceptionCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: getStringValue invalid key : ghi", e.getMessage().c_str());
        EXPECT_FALSE(e.getCause().isNull());
    }
}

TEST_F(ContentValuesUnitTest, getKeys_ReturnsZeroSizeKeyList_WhenContentValuesIsInInitialState)
{
    // Given : create ContentValues instance
    ContentValues contentValues;

    // Then: number of keys stored in ContentValues object is zero in initial state.
    std::vector<std::string> keys;
    contentValues.getKeys(keys);
    EXPECT_EQ(0, keys.size());
}

TEST_F(ContentValuesUnitTest, getKeys_ReturnsListOfAllStoredKeys)
{
    // Given : create ContentValues instance
    ContentValues contentValues;

    // When: put data
    std::string keys_array[] = {"abc", "def", "ghi"};
    contentValues.put(keys_array[0], "test A");
    contentValues.put(keys_array[1], "test B");
    contentValues.put(keys_array[2], "test C");

    // get key list
    std::vector<std::string> keys;
    contentValues.getKeys(keys);

    // Then: number of keys stored in ContentValues object is equals to
    // the number that have put.
    ASSERT_EQ(3, keys.size()) << "The number of key list stored in ContentValues object is not valid";

    // the key string can obtain from key list
    for (size_t i = 0; i < keys.size(); i++) {
        EXPECT_STREQ(keys_array[i].c_str(), keys.at(i).c_str());
    }
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
