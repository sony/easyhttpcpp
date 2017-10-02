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

TEST(EasyHttpUnitTest, newCall_ReturnsCallInstance)
{
    // Given: none
    EasyHttp::Builder builder;
    EasyHttp::Ptr pEasyHttp = builder.build();

    Request::Builder requestBuilder;
    Request::Ptr pRequest = requestBuilder.httpGet().setUrl(Url).build();

    // When: call newCall()
    Call::Ptr pCall = pEasyHttp->newCall(pRequest);

    // Then: returns Call instance
    EXPECT_FALSE(pCall.isNull());
    EXPECT_EQ(pRequest, pCall->getRequest());
}

TEST(EasyHttpUnitTest, newCall_ThrowsHttpIllegalArgumentException_WhenRequestIsNull)
{
    // Given: none
    EasyHttp::Builder builder;
    EasyHttp::Ptr pEasyHttp = builder.build();

    // When: call newCall()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(pEasyHttp->newCall(NULL), HttpIllegalArgumentException, 100700);
}

TEST(EasyHttpBuilderUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    // When: call Builder()
    EasyHttp::Builder builder;

    // Then: initial value is set
    EXPECT_TRUE(builder.getProxy().isNull());
    EXPECT_EQ(EasyHttpContext::DefaultTimeoutSec, builder.getTimeoutSec());
    EXPECT_TRUE(builder.getCache().isNull());
    EXPECT_EQ(CrlCheckPolicyNoCheck, builder.getCrlCheckPolicy());
    EXPECT_EQ("", builder.getRootCaDirectory());
    EXPECT_EQ("", builder.getRootCaFile());
    EXPECT_TRUE(builder.getInterceptors().empty());
    EXPECT_TRUE(builder.getNetworkInterceptors().empty());
}

TEST(EasyHttpBuilderUnitTest, build_ReturnsEasyHttpInstance)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call build()
    EasyHttp::Ptr pEasyHttp = builder.build();

    // Then: default value is set
    EXPECT_TRUE(pEasyHttp->getProxy().isNull());
    EXPECT_EQ(EasyHttpContext::DefaultTimeoutSec, pEasyHttp->getTimeoutSec());
    EXPECT_TRUE(pEasyHttp->getCache().isNull());
    EXPECT_EQ(CrlCheckPolicyNoCheck, pEasyHttp->getCrlCheckPolicy());
    EXPECT_EQ("", pEasyHttp->getRootCaDirectory());
    EXPECT_EQ("", pEasyHttp->getRootCaFile());
}

TEST(EasyHttpBuilderUnitTest, setProxy_StoresProxy)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setProxy()
    Proxy::Ptr pProxy = new Proxy(ProxyHostName, ProxyPort);
    builder.setProxy(pProxy);

    // Then: stores proxy
    EXPECT_EQ(pProxy, builder.getProxy());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(pProxy, pEasyHttp->getProxy());
}

TEST(EasyHttpBuilderUnitTest, setTimeoutSec_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setTimeoutSec()
    builder.setTimeoutSec(TimeoutSec);

    // Then: stores value
    EXPECT_EQ(TimeoutSec, builder.getTimeoutSec());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(TimeoutSec, pEasyHttp->getTimeoutSec());
}

TEST(EasyHttpBuilderUnitTest, setTimeoutSec_ThrowsHttpIllegalArgumentException_WhenValueIs0)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setTimeoutSec()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setTimeoutSec(0), HttpIllegalArgumentException, 100700);
}

TEST(EasyHttpBuilderUnitTest, setCache_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;
    Poco::Path path(CachePath);

    // When: call setCache()
    HttpCache::Ptr pCache = HttpCache::createCache(path, CacheMaxSize);
    builder.setCache(pCache);

    // Then: stores value
    EXPECT_EQ(pCache, builder.getCache());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(pCache, pEasyHttp->getCache());
}

TEST(EasyHttpBuilderUnitTest, setCrlCheckPolicy_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setCrlCheckPolicy()
    builder.setCrlCheckPolicy(CrlCheckPolicyCheckHardFail);

    // Then: stores value
    EXPECT_EQ(CrlCheckPolicyCheckHardFail, builder.getCrlCheckPolicy());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(CrlCheckPolicyCheckHardFail, pEasyHttp->getCrlCheckPolicy());
}

TEST(EasyHttpBuilderUnitTest, setRootCaDirectory_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setRootCaDirectory()
    builder.setRootCaDirectory(CaDirectoryName);

    // Then: stores value
    EXPECT_EQ(CaDirectoryName, builder.getRootCaDirectory());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(CaDirectoryName, pEasyHttp->getRootCaDirectory());
}

TEST(EasyHttpBuilderUnitTest, setRootCaDirectory_ThrowsHttpIllegalArgumentException_WhenValueIsEmpty)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setRootCaDirectory()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setRootCaDirectory(""), HttpIllegalArgumentException, 100700);
}

TEST(EasyHttpBuilderUnitTest, setRootCaFile_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setRootCaFile()
    builder.setRootCaFile(CaFileName);

    // Then: stores value
    EXPECT_EQ(CaFileName, builder.getRootCaFile());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(CaFileName, pEasyHttp->getRootCaFile());
}

TEST(EasyHttpBuilderUnitTest, setRootCaFile_ThrowsHttpIllegalArgumentException_WhenValueIsEmpty)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setRootCaFile()
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(builder.setRootCaFile(""), HttpIllegalArgumentException, 100700);
}

TEST(EasyHttpBuilderUnitTest, addInterceptor_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;
    Interceptor::Ptr pMockInterceptor = new easyhttpcpp::testutil::MockInterceptor();

    // When: call addInterceptor()
    builder.addInterceptor(pMockInterceptor);

    // Then: stores value
    EXPECT_FALSE(builder.getInterceptors().empty());
    EXPECT_EQ(1u, builder.getInterceptors().size());
    EXPECT_EQ(pMockInterceptor, builder.getInterceptors().front());
}

TEST(EasyHttpBuilderUnitTest, addNetworkInterceptor_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;
    Interceptor::Ptr pMockInterceptor = new easyhttpcpp::testutil::MockInterceptor();

    // When: call addNetworkInterceptor()
    builder.addNetworkInterceptor(pMockInterceptor);

    // Then: stores value
    EXPECT_FALSE(builder.getNetworkInterceptors().empty());
    EXPECT_EQ(1u, builder.getNetworkInterceptors().size());
    EXPECT_EQ(pMockInterceptor, builder.getNetworkInterceptors().front());
}

TEST(EasyHttpBuilderUnitTest, setConnectionPool_StoresValue)
{
    // Given: none
    EasyHttp::Builder builder;

    // When: call setConnectionPool()
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    EXPECT_EQ(&builder, &builder.setConnectionPool(pConnectionPool));

    // Then: stores value
    EXPECT_EQ(pConnectionPool, builder.getConnectionPool());

    EasyHttp::Ptr pEasyHttp = builder.build();
    EXPECT_EQ(pConnectionPool, pEasyHttp->getConnectionPool());
}

} /* namespace test */
} /* namespace easyhttpcpp */

