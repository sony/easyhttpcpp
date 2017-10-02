/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/Interceptor.h"
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

TEST(EasyHttpContextUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    // When: call EasyHttpContext()
    EasyHttpContext context;

    // Then: initial value is set
    EXPECT_TRUE(context.getCache().isNull());
    EXPECT_EQ(EasyHttpContext::DefaultTimeoutSec, context.getTimeoutSec());
    EXPECT_TRUE(context.getProxy().isNull());
    EXPECT_EQ("", context.getRootCaDirectory());
    EXPECT_EQ("", context.getRootCaFile());
    EXPECT_EQ(CrlCheckPolicyNoCheck, context.getCrlCheckPolicy());
    EXPECT_TRUE(context.getCallInterceptors().empty());
    EXPECT_TRUE(context.getNetworkInterceptors().empty());
}

TEST(EasyHttpContextUnitTest, setCache_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setCache()
    Poco::Path path(CachePath);
    HttpCache::Ptr pCache = HttpCache::createCache(path, CacheMaxSize);
    context.setCache(pCache);

    // Then: stores value
    EXPECT_EQ(pCache, context.getCache());
}

TEST(EasyHttpContextUnitTest, setTimeoutSec_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setTimeoutSec()
    context.setTimeoutSec(TimeoutSec);

    // Then: stores value
    EXPECT_EQ(TimeoutSec, context.getTimeoutSec());
}

TEST(EasyHttpContextUnitTest, setTimeoutSec_StoresValue_WhenValueIs0)
{
    // Given: none
    EasyHttpContext context;

    // When: call setTimeoutSec()
    context.setTimeoutSec(0);

    // Then: stores value
    EXPECT_EQ(0u, context.getTimeoutSec());
}

TEST(EasyHttpContextUnitTest, setProxy_StoresProxy)
{
    // Given: none
    EasyHttpContext context;

    // When: call setProxy()
    Proxy::Ptr pProxy = new Proxy(ProxyHostName, ProxyPort);
    context.setProxy(pProxy);

    // Then: stores proxy
    EXPECT_EQ(pProxy, context.getProxy());
}

TEST(EasyHttpContextUnitTest, setRootCaDirectory_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setRootCaDirectory()
    context.setRootCaDirectory(CaDirectoryName);

    // Then: stores value
    EXPECT_EQ(CaDirectoryName, context.getRootCaDirectory());
}

TEST(EasyHttpContextUnitTest, setRootCaDirectory_StoresValue_WhenValueIsEmpty)
{
    // Given: none
    EasyHttpContext context;

    // When: call setRootCaDirectory()
    context.setRootCaDirectory("");

    // Then: stores value
    EXPECT_EQ("", context.getRootCaDirectory());
}

TEST(EasyHttpContextUnitTest, setRootCaFile_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setRootCaFile()
    context.setRootCaFile(CaFileName);

    // Then: stores value
    EXPECT_EQ(CaFileName, context.getRootCaFile());
}

TEST(EasyHttpContextUnitTest, setRootCaFile_StoresValue_WhenValueIsEmpty)
{
    // Given: none
    EasyHttpContext context;

    // When: call setRootCaFile()
    context.setRootCaFile("");

    // Then: stores value
    EXPECT_EQ("", context.getRootCaFile());
}

TEST(EasyHttpContextUnitTest, setCrlCheckPolicy_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setCrlCheckPolicy()
    context.setCrlCheckPolicy(CrlCheckPolicyCheckHardFail);

    // Then: stores value
    EXPECT_EQ(CrlCheckPolicyCheckHardFail, context.getCrlCheckPolicy());
}

TEST(EasyHttpContextUnitTest, addCallInterceptor_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setCallInterceptors()
    Interceptor::Ptr pMockInterceptor = new easyhttpcpp::testutil::MockInterceptor();
    EasyHttpContext::InterceptorList interceptorList;
    interceptorList.push_back(pMockInterceptor);
    context.setCallInterceptors(interceptorList);

    // Then: stores value
    EXPECT_FALSE(context.getCallInterceptors().empty());
    EXPECT_EQ(1u, context.getCallInterceptors().size());
    EXPECT_EQ(pMockInterceptor, context.getCallInterceptors().front());
}

TEST(EasyHttpContextUnitTest, addNetworkInterceptor_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setNetworkInterceptors()
    Interceptor::Ptr pMockInterceptor = new easyhttpcpp::testutil::MockInterceptor();
    EasyHttpContext::InterceptorList interceptorList;
    interceptorList.push_back(pMockInterceptor);
    context.setNetworkInterceptors(interceptorList);

    // Then: stores value
    EXPECT_FALSE(context.getNetworkInterceptors().empty());
    EXPECT_EQ(1u, context.getNetworkInterceptors().size());
    EXPECT_EQ(pMockInterceptor, context.getNetworkInterceptors().front());
}

TEST(EasyHttpContextUnitTest, setConnectionPool_StoresValue)
{
    // Given: none
    EasyHttpContext context;

    // When: call setConnectionPool()
    ConnectionPool::Ptr pConnectionPool = ConnectionPool::createConnectionPool();
    context.setConnectionPool(pConnectionPool);

    // Then: stores value
    EXPECT_EQ(pConnectionPool, context.getConnectionPool());
}

} /* namespace test */
} /* namespace easyhttpcpp */

