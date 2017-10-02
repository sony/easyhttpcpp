/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/messagedigest/DigestUtil.h"

namespace easyhttpcpp {
namespace messagedigest {
namespace test {

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;

typedef std::string(*FuncDigestWithByteArrayBuffer) (const ByteArrayBuffer&);
typedef std::string(*FuncDigestWithString) (const std::string&);

class DigestUtilUnitTest : public testing::Test {
public:
    static const Byte SourceDataNormal[];
    static const Byte SourceDataUnderlyingNull[];
    static const Byte SourceDataOneByte[];
    static const Byte SourceDataNullByte;

    static const std::string ExpectedSha256StringNormal;
    static const std::string ExpectedSha256UnderlyingNull;
    static const std::string ExpectedSha256OneByte;
    static const std::string ExpectedSha256NullByteByByteArrayBuffer;
    static const std::string ExpectedSha256NullByteByString;
    static const std::string ExpectedSha256NoData;

    static const std::string ExpectedSha1StringNormal;
    static const std::string ExpectedSha1UnderlyingNull;
    static const std::string ExpectedSha1OneByte;
    static const std::string ExpectedSha1NullByteByByteArrayBuffer;
    static const std::string ExpectedSha1NullByteByString;
    static const std::string ExpectedSha1NoData;
};
const Byte DigestUtilUnitTest::SourceDataNormal[] = {0xA7, 0x8D, 0xB2, 0x9F, 0x35, 0xCD, 0x10, 0xA4, 0x5B, 0xE6};
const Byte DigestUtilUnitTest::SourceDataUnderlyingNull[] = {0xA7, 0x8D, 0x00, 0x92, 0xF0, 0x31, 0x00, 0xCD};
const Byte DigestUtilUnitTest::SourceDataOneByte[] = {'a', '\0'};
const Byte DigestUtilUnitTest::SourceDataNullByte = '\0';

const std::string DigestUtilUnitTest::ExpectedSha256StringNormal =
        "41d2212faaf1e3a37bff36a35b6b0807ce14df5c26dd91e3a0cf124a57301d30";
const std::string DigestUtilUnitTest::ExpectedSha256UnderlyingNull =
        "3ecbb8dadc8f04f2ed4603ff4b7560cedcd9e32410cb0feea4783a0c1d08475b";
const std::string DigestUtilUnitTest::ExpectedSha256OneByte =
        "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb";
const std::string DigestUtilUnitTest::ExpectedSha256NullByteByByteArrayBuffer =
        "6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d";
const std::string DigestUtilUnitTest::ExpectedSha256NullByteByString =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
const std::string DigestUtilUnitTest::ExpectedSha256NoData =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

const std::string DigestUtilUnitTest::ExpectedSha1StringNormal = "5a25b0df8e82f6bcdfc1a59719bd3f86cf4ee2b0";
const std::string DigestUtilUnitTest::ExpectedSha1UnderlyingNull = "312986580954e3e20696525aca73b45caf4f4bad";
const std::string DigestUtilUnitTest::ExpectedSha1OneByte = "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8";
const std::string DigestUtilUnitTest::ExpectedSha1NullByteByByteArrayBuffer =
        "5ba93c9db0cff93f52b521d7420e43f6eda2784f";
const std::string DigestUtilUnitTest::ExpectedSha1NullByteByString = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
const std::string DigestUtilUnitTest::ExpectedSha1NoData = "da39a3ee5e6b4b0d3255bfef95601890afd80709";

class DigestWithByteArrayBufferTestParam {
public:
    FuncDigestWithByteArrayBuffer m_pFuncWithByteArrayBuffer;
    const Byte* m_pData;
    size_t m_dataSize;
    std::string m_hashString;
};

static const DigestWithByteArrayBufferTestParam DigestWithByteArrayBufferTestParams[] =
{
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha256StringNormal},
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataUnderlyingNull,
            sizeof(DigestUtilUnitTest::SourceDataUnderlyingNull), DigestUtilUnitTest::ExpectedSha256UnderlyingNull},
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataOneByte, 1, DigestUtilUnitTest::ExpectedSha256OneByte},
    {DigestUtil::sha256Hex, &DigestUtilUnitTest::SourceDataNullByte, 1,
            DigestUtilUnitTest::ExpectedSha256NullByteByByteArrayBuffer},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha1StringNormal},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataUnderlyingNull,
            sizeof(DigestUtilUnitTest::SourceDataUnderlyingNull), DigestUtilUnitTest::ExpectedSha1UnderlyingNull},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataOneByte, 1, DigestUtilUnitTest::ExpectedSha1OneByte},
    {DigestUtil::sha1Hex, &DigestUtilUnitTest::SourceDataNullByte, 1,
            DigestUtilUnitTest::ExpectedSha1NullByteByByteArrayBuffer}
};

class DigestWithByteArrayBufferParameterizeTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithByteArrayBufferTestParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithByteArrayBufferParameterizeTest,
        ::testing::ValuesIn(DigestWithByteArrayBufferTestParams));

TEST_P(DigestWithByteArrayBufferParameterizeTest, digestHex_ReturnsDigestStringOfHexWhichIsMadeFromSourceData)
{
    //Given: 元データを準備する
    ByteArrayBuffer source;
    source.write(GetParam().m_pData, GetParam().m_dataSize);

    //When: digestを取得する
    std::string actual = GetParam().m_pFuncWithByteArrayBuffer(source);

    // Then: 期待したdigestが取得できる
    EXPECT_EQ(GetParam().m_hashString, actual);
}

class DigestWithStringTestParam {
public:
    FuncDigestWithString m_pFuncWithString;
    const Byte* m_pData;
    size_t m_dataSize;
    std::string m_hashString;
};

static const DigestWithStringTestParam DigestWithStringTestParams[] = {
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha256StringNormal},
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataOneByte, 1, DigestUtilUnitTest::ExpectedSha256OneByte},
    {DigestUtil::sha256Hex, &DigestUtilUnitTest::SourceDataNullByte, 0/*empty*/,
            DigestUtilUnitTest::ExpectedSha256NullByteByString},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha1StringNormal},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataOneByte, 1, DigestUtilUnitTest::ExpectedSha1OneByte},
    {DigestUtil::sha1Hex, &DigestUtilUnitTest::SourceDataNullByte, 0/*empty*/,
            DigestUtilUnitTest::ExpectedSha1NullByteByString}
};

class DigestWithStringParameterizedTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithStringTestParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithStringParameterizedTest,
        ::testing::ValuesIn(DigestWithStringTestParams));

TEST_P(DigestWithStringParameterizedTest, digestHex_ReturnsDigestStringOfHexWhichIsMadeFromSourceData)
{
    //Given: 元データを準備する
    std::string source(reinterpret_cast<const char*> (GetParam().m_pData), GetParam().m_dataSize);

    //When: digestを取得する
    std::string actual = GetParam().m_pFuncWithString(source);

    // Then: 期待したdigestが取得できる
    EXPECT_EQ(GetParam().m_hashString, actual);
}

class DigestWithByteArrayBufferNotInitializeParam {
public:
    FuncDigestWithByteArrayBuffer m_pFuncWithByteArrayBuffer;
    std::string m_hashString;
};

static const DigestWithByteArrayBufferNotInitializeParam DigestWithByteArrayBufferNotInitializeParams[] = {
    {DigestUtil::sha256Hex, DigestUtilUnitTest::ExpectedSha256NoData},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::ExpectedSha1NoData}
};

class DigestWithByteArrayBufferNotInitializeParameterizeTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithByteArrayBufferNotInitializeParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithByteArrayBufferNotInitializeParameterizeTest,
        ::testing::ValuesIn(DigestWithByteArrayBufferNotInitializeParams));

TEST_P(DigestWithByteArrayBufferNotInitializeParameterizeTest,
        digestHex_ReturnsDigestStringOfHex_WhenNotInitializedParameter)
{
    //Given: 初期化していないByteArrayBufferを準備する
    ByteArrayBuffer source;

    //When: digestを取得する
    std::string actual = GetParam().m_pFuncWithByteArrayBuffer(source);

    // Then: 期待したdigestが取得できる
    EXPECT_EQ(GetParam().m_hashString, actual);
}

class DigestWithStringNotInitializeParam {
public:
    FuncDigestWithString m_pFuncWithString;
    std::string m_hashString;
};

static const DigestWithStringNotInitializeParam DigestWithStringNotInitializeParams[] = {
    {DigestUtil::sha256Hex, DigestUtilUnitTest::ExpectedSha256NoData},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::ExpectedSha1NoData}
};

class DigestWithStringNotInitializeParameterizeTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithStringNotInitializeParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithStringNotInitializeParameterizeTest,
        ::testing::ValuesIn(DigestWithStringNotInitializeParams));

TEST_P(DigestWithStringNotInitializeParameterizeTest, digestHex_ReturnsDigestStringOfHex_WhenNotInitializedParameter)
{
    //Given: 初期化していないstd::stringを準備する
    std::string source;

    //When: digestを取得する
    std::string actual = GetParam().m_pFuncWithString(source);

    // Then: 期待したdigestが取得できる
    EXPECT_EQ(GetParam().m_hashString, actual);
}

class DigestWithByteArrayBufferTwiceExecutionParam {
public:
    FuncDigestWithByteArrayBuffer m_pFuncWithByteArrayBuffer;
    const Byte* m_pSourceData;
    size_t m_sourceSize;
    std::string m_hashString;
};

static const DigestWithByteArrayBufferTwiceExecutionParam DigestWithByteArrayBufferTwiceExecutionParams[] = {
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha256NoData},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha1NoData}
};

class DigestWithByteArrayBufferTwiceParameterizeTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithByteArrayBufferTwiceExecutionParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithByteArrayBufferTwiceParameterizeTest,
        ::testing::ValuesIn(DigestWithByteArrayBufferTwiceExecutionParams));

TEST_P(DigestWithByteArrayBufferTwiceParameterizeTest,
        digestHex_CanGetsSameDigestStringOfHex_WhenTwiceExecutsAtSameData)
{
    //Given: 元データを準備する
    ByteArrayBuffer data;
    data.write(GetParam().m_pSourceData, GetParam().m_sourceSize);

    //When: 同じデータで２回digestを取得する
    std::string actual1 = GetParam().m_pFuncWithByteArrayBuffer(data);
    std::string actual2 = GetParam().m_pFuncWithByteArrayBuffer(data);

    //Then: 1回目と2回目のdigestデータは同じである
    EXPECT_EQ(actual1, actual2);
}

class DigestWithStringTwiceExecutionParam {
public:
    FuncDigestWithString m_pFuncWithString;
    const Byte* m_pSourceData;
    size_t m_sourceSize;
    std::string m_hashString;
};
static const DigestWithStringTwiceExecutionParam DigestWithStringTwiceExecutionParams[] = {
    {DigestUtil::sha256Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha256NoData},
    {DigestUtil::sha1Hex, DigestUtilUnitTest::SourceDataNormal, sizeof(DigestUtilUnitTest::SourceDataNormal),
            DigestUtilUnitTest::ExpectedSha1NoData}
};

class DigestWithStringTwiceParameterizeTest : public DigestUtilUnitTest,
        public ::testing::WithParamInterface<DigestWithStringTwiceExecutionParam> {
};
INSTANTIATE_TEST_CASE_P(DigestUtilUnitTest, DigestWithStringTwiceParameterizeTest,
        ::testing::ValuesIn(DigestWithStringTwiceExecutionParams));

TEST_P(DigestWithStringTwiceParameterizeTest, digestHex_CanGetsSameDigestStringOfHex_WhenTwiceExecutsAtSameData)
{
    //Given: 元データを準備する
    std::string data(reinterpret_cast<const char*> (GetParam().m_pSourceData));

    //When: 同じデータで２回digestを取得する
    std::string actual1 = GetParam().m_pFuncWithString(data);
    std::string actual2 = GetParam().m_pFuncWithString(data);

    //Then: 1回目と2回目のdigestデータは同じである
    EXPECT_EQ(actual1, actual2);
}

} /* namespace test */
} /* namespace messagedigest */
} /* namespace easyhttpcpp */
