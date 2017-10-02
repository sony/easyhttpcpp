/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/common/Cache.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"
#include "MockInterceptor.h"

#include "EasyHttpContext.h"
#include "EasyHttpInternal.h"

namespace easyhttpcpp {
namespace test {

static const std::string Url = "http://www.example.com/path/index.html";
static const std::string ProxyHostName = "example.com";
static const unsigned short ProxyPort = 8080;
static const unsigned int TimeoutSec = EasyHttpContext::DefaultTimeoutSec * 10;
static const std::string CachePath = "/test/cache/path";
static const size_t CacheMaxSize = 1024 * 1024;
static const std::string CaDirectoryName = "foo";
static const std::string CaFileName = "bar.ca";

TEST(EasyHttpInternalUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    Proxy::Ptr pProxy = new Proxy(ProxyHostName, ProxyPort);

    Poco::Path path(CachePath);
    HttpCache::Ptr pCache = HttpCache::createCache(path, CacheMaxSize);

    Interceptor::Ptr pMockInterceptor = new easyhttpcpp::testutil::MockInterceptor();
    Interceptor::Ptr pMockNetworkInterceptor = new easyhttpcpp::testutil::MockInterceptor();

    EasyHttp::Builder builder;
    builder.setProxy(pProxy)
            .setTimeoutSec(TimeoutSec)
            .setCache(pCache)
            .setCrlCheckPolicy(CrlCheckPolicyCheckHardFail)
            .setRootCaDirectory(CaDirectoryName)
            .setRootCaFile(CaFileName)
            .addInterceptor(pMockInterceptor)
            .addNetworkInterceptor(pMockNetworkInterceptor);

    // When: call EasyHttpInternal()
    EasyHttp::Ptr pEasyHttpInternal = new EasyHttpInternal(builder);

    // Then: store values and instances
    EXPECT_FALSE(pEasyHttpInternal->getProxy().isNull());
    EXPECT_EQ(pProxy, pEasyHttpInternal->getProxy());
    EXPECT_EQ(TimeoutSec, pEasyHttpInternal->getTimeoutSec());
    EXPECT_FALSE(pEasyHttpInternal->getCache().isNull());
    EXPECT_EQ(pCache, pEasyHttpInternal->getCache());
    EXPECT_EQ(CrlCheckPolicyCheckHardFail, pEasyHttpInternal->getCrlCheckPolicy());
    EXPECT_EQ(CaDirectoryName, pEasyHttpInternal->getRootCaDirectory());
    EXPECT_EQ(CaFileName, pEasyHttpInternal->getRootCaFile());
}

TEST(EasyHttpInternalUnitTest, newCall_ReturnsCallInstance)
{
    // Given: none
    EasyHttp::Builder builder;
    EasyHttp::Ptr pEasyHttpInternal = new EasyHttpInternal(builder);

    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpGet().setUrl(Url).build();

    // When: call newCall()
    Call::Ptr pCall = pEasyHttpInternal->newCall(pRequest);

    // Then: returns Call instance
    EXPECT_FALSE(pCall.isNull());
    EXPECT_EQ(pRequest, pCall->getRequest());
}

TEST(EasyHttpInternalUnitTest, newCall_ThrowsHttpIllegalArgumentException_WhenRequestIsNull)
{
    // Given: none
    EasyHttp::Builder builder;
    EasyHttp::Ptr pEasyHttpInternal = new EasyHttpInternal(builder);

    // When: call newCall()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pEasyHttpInternal->newCall(NULL), HttpIllegalArgumentException, 100700);
}

} /* namespace test */
} /* namespace easyhttpcpp */

