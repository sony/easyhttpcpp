/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Call.h"
#include "EasyHttpCppAssertions.h"

namespace easyhttpcpp {
namespace test {

static const char* const RequestUrl = "http://quiver.com/test1";

class CallInternalUnitTest : public testing::Test {
};

TEST(CallInternalUnitTest, isCancelled_ReturnsTrue_WhenAfterCancel)
{
    // Given: call cancel
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(RequestUrl).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);
    pCall->cancel();

    // When: isCancelled.
    // Then: return true
    EXPECT_TRUE(pCall->isCancelled());
}

TEST(CallInternalUnitTest, isCancelled_ReturnsFalse_WhenBeforeCancel)
{
    // Given: create call
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(RequestUrl).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: isCancelled.
    // Then: return false
    EXPECT_FALSE(pCall->isCancelled());
}

TEST(CallInternalUnitTest, getRequest_ReturnsRequest_WhenRequestIsSpecifiedNewCall)
{
    // Given: create call with request
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    void* pTag = reinterpret_cast<void*>(123);
    Request::Ptr pRequest = requestBuilder.setUrl(RequestUrl).setTag(pTag).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: getRequest.
    Request::Ptr pGottenRequest = pCall->getRequest();

    // Then: return Specified Request on newCall.
    EXPECT_EQ(pRequest, pGottenRequest);
}

TEST(CallIneternalUnitTest, executeAsync_ThrowsHttpIlleagalArgumentException_WhenResponseCallbackIsNull)
{
    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.setUrl(RequestUrl).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // When: executeAsync with ResponseCallback as NULL.
    // Then: throws HttpIllegalArgumentException.
    EASYHTTPCPP_EXPECT_THROW(pCall->executeAsync(NULL), HttpIllegalArgumentException, 100700);
}

} /* namespace test */
} /* namespace easyhttpcpp */
