/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"
#include "MockResponseBodyStream.h"

#include "ResponseBodyStreamInternal.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace test {

static const std::string ContentType = "text/plain";
static const std::string Content = "test content data";

TEST(ResponseBodyUnitTest, create_ReturnsInstance)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);

    // When: call create()
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);

    // Then: parameters are set from content
    EXPECT_FALSE(pResponseBody.isNull());
    EXPECT_EQ(pBodyStream, pResponseBody->getByteStream());
    EXPECT_TRUE(pResponseBody->hasContentLength());
    EXPECT_EQ(Content.length(), pResponseBody->getContentLength());
    EXPECT_EQ(pMediaType, pResponseBody->getMediaType());
}

TEST(ResponseBodyUnitTest, close_CanNotReadFromBody)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);

    // When: call close()
    pResponseBody->close();

    // Then: can not read from stream
    char buffer[1024];
    EASYHTTPCPP_EXPECT_THROW(pResponseBody->getByteStream()->read(buffer, 1024), HttpIllegalStateException, 100701);
}

TEST(ResponseBodyUnitTest, toString_ReturnsStringAsContent)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);

    // When: call toString()
    // Then: returns string as content
    EXPECT_EQ(Content, pResponseBody->toString());
}

TEST(ResponseBodyUnitTest, toString_ReturnsStringAsEmpty_WhenContentLengthIs0)
{
    // Given: content length is 0
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is("");
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, 0, pBodyStream);

    // When: call toString()
    // Then: returns string as empty
    EXPECT_EQ("", pResponseBody->toString());
}

TEST(ResponseBodyUnitTest, toString_ReturnsStringAsEmpty_WhenReadSizeOfContentIs0)
{
    // Given: set up mock
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    easyhttpcpp::testutil::MockResponseBodyStream *pMockBodyStream
            = new easyhttpcpp::testutil::MockResponseBodyStream();
    EXPECT_CALL(*pMockBodyStream, close()).Times(2);
    EXPECT_CALL(*pMockBodyStream, read(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*pMockBodyStream, isEof()).Times(2).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    ResponseBodyStream::Ptr pBodyStream = pMockBodyStream;

    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, 100, pBodyStream);

    // When: call toString()
    // Then: returns string as empty
    EXPECT_EQ("", pResponseBody->toString());
}

TEST(ResponseBodyUnitTest, toString_ThrowsHttpIllegalStateException_WhenAlreadyColosed)
{
    // Given: ResponseBody is already closed
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);

    pResponseBody->close();

    // When: call toString()
    // Then: throws exception
    EASYHTTPCPP_EXPECT_THROW(pResponseBody->toString(), HttpIllegalStateException, 100701);
}

TEST(ResponseBodyUnitTest, toString_ThrowsHttpIllegalStateException_WhenAreadyCalledToString)
{
    // Given: call toString()
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, Content.length(), pBodyStream);

    EXPECT_EQ(Content, pResponseBody->toString());

    // When: call toString()
    // Then: throws exception
    EASYHTTPCPP_EXPECT_THROW(pResponseBody->toString(), HttpIllegalStateException, 100701);
}

TEST(ResponseBodyUnitTest, toString_ReturnsStringAsContent_WhenContentLengthIsLessThan0)
{
    // Given: none
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    std::istringstream is(Content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, -1, pBodyStream);

    // When: call toString()
    // Then: returns string as content
    EXPECT_EQ(Content, pResponseBody->toString());
}

TEST(ResponseBodyUnitTest, toString_ReturnsStringAsContent_WhenContentLengthIsLessThan0AndLargeData)
{
    // Given: large (10000 bytes) string
    MediaType::Ptr pMediaType(new MediaType(ContentType));
    size_t stringSize = 10000;
    const Byte* pContentUnit = reinterpret_cast<const Byte*>("0123456789");
    ByteArrayBuffer buffer(stringSize);
    for (int i = 0; i < 1000; i++) {
        buffer.write(pContentUnit, 10);
    }
    std::string content = buffer.toString();
    std::istringstream is(content);
    ResponseBodyStream::Ptr pBodyStream = new ResponseBodyStreamInternal(is);
    ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, true, -1, pBodyStream);

    // When: call toString()
    // Then: returns string as content
    EXPECT_EQ(content, pResponseBody->toString());
}

} /* namespace test */
} /* namespace easyhttpcpp */
