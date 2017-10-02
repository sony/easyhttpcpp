/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"

namespace easyhttpcpp {
namespace common {
namespace test {

namespace {
const std::string TestData = "abcdefghijklmn1234567890opqestuvexyz";
}

TEST(ByteArrayBufferUnitTest, getXXX_ReturnsInitialValue_WhenInitialized)
{
    size_t size = 1024;
    ByteArrayBuffer buffer(size);

    char* pGetBuf = (char*) buffer.getBuffer();
    char pCompData[1024] = {0};
    ASSERT_STREQ(pCompData, pGetBuf);
    ASSERT_EQ((size_t) 0, buffer.getWrittenDataSize());
    ASSERT_EQ(size, buffer.getBufferSize());
}

TEST(ByteArrayBufferUnitTest, getBuffer_ReturnsWrittenData)
{
    size_t size = 1024;
    ByteArrayBuffer buffer(size);

    // Test (Empty Data)
    Byte* pGetBuf = buffer.getBuffer();
    Byte pCompData[1024] = {0};
    char compData[1024];
    char getBuf[1024];
    snprintf(compData, 1024, "%s", pCompData);
    snprintf(getBuf, 1024, "%s", pGetBuf);
    ASSERT_STREQ(compData, getBuf);

    // Test (Test Data)
    const char* testdata = "Test Data";
    strncpy((char*) pGetBuf, testdata, size);
    ASSERT_STREQ(testdata, (char*) buffer.getBuffer());
}

TEST(ByteArrayBufferUnitTest, getWrittenDataSize_ReturnsWrittenDataSizeThatSetBySetWrittenDataSize)
{
    size_t size = 1024;
    ByteArrayBuffer buffer(size);

    char* pGetBuf = (char*) buffer.getBuffer();
    const char* testdata = "Test Data";
    size_t dataLen = strlen(testdata);
    strncpy(pGetBuf, testdata, size);
    buffer.setWrittenDataSize(dataLen);
    // Test
    ASSERT_EQ(dataLen, buffer.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, constructor_CreatesByteArrayBufferObject_WhenParameterIsString)
{
    // Given : setup string data
    std::string str = TestData;

    // When : create ByteArrayBuffer by String
    ByteArrayBuffer buf(str);

    // Then : data is same
    EXPECT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
    EXPECT_EQ(str.length(), buf.getWrittenDataSize());

    // clean up
    buf.clear();
}

TEST(ByteArrayBufferUnitTest, constructor_CreatesByteArrayBufferObject_WhenParameterIsEmptyString)
{
    // Given : setup string data
    std::string str = "";

    // When : create ByteArrayBuffer by String
    ByteArrayBuffer buf(str);

    // Then : data is same
    EXPECT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
    EXPECT_EQ(str.length(), buf.getWrittenDataSize());

    // clean up
    buf.clear();
}

TEST(ByteArrayBufferUnitTest, write_Succeeds_WhenTheDataToBeWrittenIsLargerThanBufferSize)
{
    // Given : setup ByteArrayBuffer with size 1
    size_t size = 1;
    ByteArrayBuffer buffer(size);

    char* pData = new char[256];
    memset(pData, 0, 256);
    strncpy(pData, TestData.c_str(), TestData.size());

    // When : write data to buffer
    EXPECT_EQ(TestData.size() + 1, buffer.write(reinterpret_cast<Byte*> (pData), TestData.size() + 1));

    // Then : data is same
    EXPECT_STREQ(TestData.c_str(), reinterpret_cast<char*> (buffer.getBuffer()));

    delete [] pData;
    pData = NULL;
}

TEST(ByteArrayBufferUnitTest, write_Succeeds_WhenTheDataToBeWrittenIsSmallerThanBufferSize)
{
    // Given : setup ByteArrayBuffer with size 1
    size_t size = 1024;
    ByteArrayBuffer buffer(size);

    char* pData = new char[256];
    memset(pData, 0, 256);
    strncpy(pData, TestData.c_str(), TestData.size());

    // When : write data to buffer
    EXPECT_EQ(TestData.size() + 1, buffer.write(reinterpret_cast<Byte*> (pData), TestData.size() + 1));

    // Then : data is same
    EXPECT_STREQ(TestData.c_str(), reinterpret_cast<char*> (buffer.getBuffer()));

    delete [] pData;
    pData = NULL;
}

TEST(ByteArrayBufferUnitTest, clear_Succeeds)
{
    // Given : create ByteArrayBuffer
    std::string str = TestData;
    ByteArrayBuffer buf(str);
    ASSERT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
    ASSERT_EQ(str.length(), buf.getWrittenDataSize());

    // When : call clear
    buf.clear();

    // Then : size is zero
    EXPECT_EQ(reinterpret_cast<Byte*> (NULL), buf.getBuffer());
    EXPECT_EQ(static_cast<size_t> (0), buf.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, clear_Succeeds_WhenByteArrayBufferIsEmptyInInitialState)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf;

    // When : call clear
    buf.clear();

    // Then : size is zero
    EXPECT_EQ(reinterpret_cast<Byte*> (NULL), buf.getBuffer());
    EXPECT_EQ(static_cast<size_t> (0), buf.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, toString_Succeeds)
{
    // Given : create ByteArrayBuffer
    std::string str = TestData;
    ByteArrayBuffer buf(str);
    ASSERT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
    ASSERT_EQ(str.length(), buf.getWrittenDataSize());

    // When : call toString
    std::string ret = buf.toString();

    // Then : string is same
    EXPECT_STREQ(str.c_str(), ret.c_str());
    EXPECT_EQ(str.length(), ret.length());
}

TEST(ByteArrayBufferUnitTest, toString_Succeeds_InInitialState)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf;
    ASSERT_EQ(reinterpret_cast<Byte*> (NULL), buf.getBuffer());
    ASSERT_EQ(static_cast<size_t> (0), buf.getWrittenDataSize());

    // When : call toString
    std::string ret = buf.toString();

    // Then : string is empty
    EXPECT_STREQ("", ret.c_str());
    EXPECT_EQ(static_cast<size_t> (0), ret.length());
}

TEST(ByteArrayBufferUnitTest, copyTo_Succeeds)
{
    // Given : create ByteArrayBuffer
    std::string str = TestData;
    ByteArrayBuffer buf1(str);
    ASSERT_STREQ(str.c_str(), reinterpret_cast<char*> (buf1.getBuffer()));
    ASSERT_EQ(str.length(), buf1.getWrittenDataSize());

    // When : call copyTo
    ByteArrayBuffer buf2;
    buf1.copyTo(buf2);

    // Then : ByteArrayBuffer is same
    EXPECT_STREQ(reinterpret_cast<char*> (buf1.getBuffer()), reinterpret_cast<char*> (buf2.getBuffer()));
    EXPECT_EQ(buf1.getWrittenDataSize(), buf2.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, copyTo_Succeeds_WhenSourceByteArrayBufferIsInInitialState)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf1;
    ASSERT_EQ(reinterpret_cast<Byte*> (NULL), buf1.getBuffer());
    ASSERT_EQ(static_cast<size_t> (0), buf1.getWrittenDataSize());

    // When : call copyTo
    ByteArrayBuffer buf2;
    buf1.copyTo(buf2);

    // Then : ByteArrayBuffer is same
    EXPECT_STREQ(reinterpret_cast<char*> (buf1.getBuffer()), reinterpret_cast<char*> (buf2.getBuffer()));
    EXPECT_EQ(buf1.getWrittenDataSize(), buf2.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, copyFrom_Succeeds_WhenSourceIsString)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf;
    ASSERT_EQ(reinterpret_cast<Byte*> (NULL), buf.getBuffer());
    ASSERT_EQ(static_cast<size_t> (0), buf.getWrittenDataSize());

    // When : call copyFrom
    std::string str = TestData;
    buf.copyFrom(str);

    // When : data is same
    EXPECT_EQ(str.length(), buf.getWrittenDataSize());
    EXPECT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
}

TEST(ByteArrayBufferUnitTest, copyFrom_Succeeds_WhenSourceIsEmptyString)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf;
    ASSERT_EQ(reinterpret_cast<Byte*> (NULL), buf.getBuffer());
    ASSERT_EQ(static_cast<size_t> (0), buf.getWrittenDataSize());

    // When : call copyFrom
    std::string str = "";
    buf.copyFrom(str);

    // When : data is empty
    EXPECT_EQ(str.length(), buf.getWrittenDataSize());
    EXPECT_STREQ(str.c_str(), reinterpret_cast<char*> (buf.getBuffer()));
}

TEST(ByteArrayBufferUnitTest, copyFrom_Succeeds_WhenSourceIsByteArrayBuffer)
{
    // Given : create ByteArrayBuffer
    std::string str = TestData;
    ByteArrayBuffer buf1(str);
    ASSERT_STREQ(str.c_str(), reinterpret_cast<char*> (buf1.getBuffer()));
    ASSERT_EQ(str.length(), buf1.getWrittenDataSize());

    // When : call copyFrom
    ByteArrayBuffer buf2;
    buf2.copyFrom(buf1);

    // Then : ByteArrayBuffer is same
    EXPECT_STREQ(reinterpret_cast<char*> (buf1.getBuffer()), reinterpret_cast<char*> (buf1.getBuffer()));
    EXPECT_EQ(buf1.getWrittenDataSize(), buf2.getWrittenDataSize());
}

TEST(ByteArrayBufferUnitTest, copyFrom_Succeeds_WhenSourceIsByteArrayBufferThatIsInInitialState)
{
    // Given : create ByteArrayBuffer
    ByteArrayBuffer buf1;
    ASSERT_EQ(reinterpret_cast<Byte*> (NULL), buf1.getBuffer());
    ASSERT_EQ(static_cast<size_t> (0), buf1.getWrittenDataSize());

    // When : call copyFrom
    ByteArrayBuffer buf2;
    buf2.copyFrom(buf1);

    // Then : ByteArrayBuffer is same
    EXPECT_EQ(reinterpret_cast<Byte*> (NULL), buf2.getBuffer());
    EXPECT_EQ(static_cast<size_t> (0), buf2.getWrittenDataSize());
    EXPECT_STREQ(reinterpret_cast<char*> (buf1.getBuffer()), reinterpret_cast<char*> (buf1.getBuffer()));
    EXPECT_EQ(buf1.getWrittenDataSize(), buf2.getWrittenDataSize());
}

} /* namespace test */
} /* namespace common */
} /* namespace easyhttpcpp */
