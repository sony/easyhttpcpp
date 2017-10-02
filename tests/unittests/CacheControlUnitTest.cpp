/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>
#include <sys/resource.h>

#include "gtest/gtest.h"

#include "easyhttpcpp/CacheControl.h"
#include "easyhttpcpp/Headers.h"

namespace easyhttpcpp {
namespace test {

TEST(CacheControlUnitTest, createForceCache_ReturnsInstanceWithDefaultValues)
{
    // Given: none
    // When: call createForceCache()
    CacheControl::Ptr pCacheControl = CacheControl::createForceCache();

    // Then: parameters are set as default
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(LLONG_MAX, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_TRUE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createForceNetwork_ReturnsInstanceWithDefaultValues)
{
    // Given: none
    // When: call createForceNetwork()
    CacheControl::Ptr pCacheControl = CacheControl::createForceNetwork();

    // Then: parameters are set as default
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstanceWithDefaultValues_WhenHeadersAreEmpty)
{
    // Given: set up headers  
    Headers::Ptr pHeaders = new Headers();

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set as default
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstanceWithDefaultValues_WhenHeadersAreNull)
{
    // Given: none
    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(NULL);

    // Then: parameters are set as default
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_PragmaHeaderIncludesNoCache)
{
    // Given: set up headers
    const std::string headerValue = "no-cache";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Pragma", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_PragmaHeaderNotIncludesNoCache)
{
    // Given: set up headers
    const std::string headerValue = "foo";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Pragma", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesPublicDirective)
{
    // Given: set up headers
    const std::string headerValue = "public";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_TRUE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesPrivateDirective)
{
    // Given: set up headers
    const std::string headerValue = "private";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesPrivateDirectiveWithName)
{
    // Given: set up headers
    const std::string headerValue = "private=field-name";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesPrivateDirectiveWithNamesByQuotedString)
{
    // Given: set up headers
    const std::string headerValue = "private=\"field-name1, field-name2\"";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesNoCacheDirective)
{
    // Given: set up headers
    const std::string headerValue = "no-cache";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesNoCacheDirectiveWithName)
{
    // Given: set up headers
    const std::string headerValue = "no-cache=field-name";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesNoCacheDirectiveWithNamesByQuotedString)
{
    // Given: set up headers
    const std::string headerValue = "no-cache=\"field-name1, field-name2\"";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesNoStoreDirective)
{
    // Given: set up headers
    const std::string headerValue = "no-store";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_TRUE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesMaxAgeDirective)
{
    // Given: set up headers
    const std::string headerValue = "max-age=10";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(10, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesSMaxAgeDirective)
{
    // Given: set up headers
    const std::string headerValue = "s-maxage=10";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(10, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesMustRevalidateDirective)
{
    // Given: set up headers
    const std::string headerValue = "must-revalidate";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_TRUE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesMaxStaleDirective)
{
    // Given: set up headers
    const std::string headerValue = "max-stale=10";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(10, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesMinFreshDirective)
{
    // Given: set up headers
    const std::string headerValue = "min-fresh=10";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(10, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesOnlyIfCachedDirective)
{
    // Given: set up headers
    const std::string headerValue = "only-if-cached";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_TRUE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesNoTransformDirective)
{
    // Given: set up headers
    const std::string headerValue = "no-transform";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_TRUE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstanceAndIgnoreExtensions_CacheControlHeaderIncludesExtensions)
{
    // Given: set up headers
    // Example from http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.9.6
    const std::string headerValue = "private, community=\"UCI\"";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderIncludesAllDirectives)
{
    // Given: set up headers
    const std::string headerValue = "no-cache, no-store, max-age=1, s-maxage=2, private, public, must-revalidate, max-stale=3, min-fresh=4, only-if-cached, no-transform";
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", headerValue);

    // When: call createFromHeaders()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(3, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(4, pCacheControl->getMinFreshSec());
    EXPECT_EQ(2, pCacheControl->getSMaxAgeSec());
    EXPECT_TRUE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_TRUE(pCacheControl->isNoStore());
    EXPECT_TRUE(pCacheControl->isNoTransform());
    EXPECT_TRUE(pCacheControl->isOnlyIfCached());
    EXPECT_TRUE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlAndPragmaHeadersAreCombined)
{
    // Given: set up headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", "max-age=12");
    pHeaders->add("Pragma", "must-revalidate");
    pHeaders->add("Pragma", "public");

    // When: call createForceNetwork()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(12, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderValueInvalidatedByPragma)
{
    // Given: set up headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", "max-age=12");
    pHeaders->add("Pragma", "must-revalidate");

    // When: call createForceNetwork()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(12, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlUnitTest, createFromHeaders_ReturnsInstance_CacheControlHeaderValueInvalidatedByTwoValues)
{
    // Given: set up headers
    Headers::Ptr pHeaders = new Headers();
    pHeaders->add("Cache-Control", "max-age=12");
    pHeaders->add("Cache-Control", "must-revalidate");

    // When: call createForceNetwork()
    CacheControl::Ptr pCacheControl = CacheControl::createFromHeaders(pHeaders);

    // Then: parameters are set from headers
    EXPECT_EQ(12, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_TRUE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, constructor_SetsAllPropertiesToDefault)
{
    // Given: none
    // When: call Builder()
    CacheControl::Builder builder;

    // Then: parameters are set as default
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
}

TEST(CacheControlBuilderUnitTest, build_ReturnsInstance)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call build()
    CacheControl::Ptr pCacheControl = builder.build();

    // Then: parameters are set as default
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, build_ReturnsInstance_WhenAllParametersAreSet)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call build()
    CacheControl::Ptr pCacheControl = builder
            .setMaxAgeSec(10)
            .setMaxStaleSec(20)
            .setMinFreshSec(30)
            .setSMaxAgeSec(40)
            .setMustRevalidate(true)
            .setNoCache(true)
            .setNoStore(true)
            .setNoTransform(true)
            .setOnlyIfCached(true)
            .setPublic(true)
            .setPrivate(true).build();

    // Then: parameters are set as default
    EXPECT_EQ(10, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(20, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(30, pCacheControl->getMinFreshSec());
    EXPECT_EQ(40, pCacheControl->getSMaxAgeSec());
    EXPECT_TRUE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_TRUE(pCacheControl->isNoStore());
    EXPECT_TRUE(pCacheControl->isNoTransform());
    EXPECT_TRUE(pCacheControl->isOnlyIfCached());
    EXPECT_TRUE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxAgeSec_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxAgeSec()
    builder.setMaxAgeSec(10);

    // Then: value is stored
    EXPECT_EQ(10, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(10, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxAgeSec_StoreValue_WhenValueIsMax)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxAgeSec()
    builder.setMaxAgeSec(LLONG_MAX);

    // Then: value is stored
    EXPECT_EQ(LLONG_MAX, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(LLONG_MAX, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxAgeSec_StoreValue_WhenValueIsMin)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxAgeSec()
    builder.setMaxAgeSec(LLONG_MIN);

    // Then: value is stored
    EXPECT_EQ(LLONG_MIN, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(LLONG_MIN, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxStaleSec_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxStaleSec()
    builder.setMaxStaleSec(10);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(10, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(10, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxStaleSec_StoreValue_WhenValueIsMax)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxStaleSec()
    builder.setMaxStaleSec(LLONG_MAX);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(LLONG_MAX, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(LLONG_MAX, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMaxStaleSec_StoreValue_WhenValueIsMin)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMaxStaleSec()
    builder.setMaxStaleSec(LLONG_MIN);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(LLONG_MIN, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(LLONG_MIN, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMinFreshSec_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMinFreshSec()
    builder.setMinFreshSec(10);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(10, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(10, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMinFreshSec_StoreValue_WhenValueIsMax)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMinFreshSec()
    builder.setMinFreshSec(LLONG_MAX);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(LLONG_MAX, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(LLONG_MAX, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMinFreshSec_StoreValue_WhenValueIsMin)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMinFreshSec()
    builder.setMinFreshSec(LLONG_MIN);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(LLONG_MIN, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(LLONG_MIN, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setSMaxAgeSec_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setSMaxAgeSec()
    builder.setSMaxAgeSec(10);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(10, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(10, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setSMaxAgeSec_StoreValue_WhenValueIsMax)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setSMaxAgeSec()
    builder.setSMaxAgeSec(LLONG_MAX);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(LLONG_MAX, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(LLONG_MAX, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setSMaxAgeSec_StoreValue_WhenValueIsMin)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setSMaxAgeSec()
    builder.setSMaxAgeSec(LLONG_MIN);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(LLONG_MIN, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(LLONG_MIN, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setMustRevalidate_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setMustRevalidate()
    builder.setMustRevalidate(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_TRUE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_TRUE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setNoCache_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setNoCache()
    builder.setNoCache(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_TRUE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_TRUE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setNoTransform_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setNoTransform()
    builder.setNoTransform(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_TRUE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_TRUE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setOnlyIfCached_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setOnlyIfCached()
    builder.setOnlyIfCached(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_TRUE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_TRUE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setPublic_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setPublic()
    builder.setPublic(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_TRUE(builder.isPublic());
    EXPECT_FALSE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_TRUE(pCacheControl->isPublic());
    EXPECT_FALSE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

TEST(CacheControlBuilderUnitTest, setPrivate_StoreValue)
{
    // Given: none
    CacheControl::Builder builder;

    // When: call setPrivate()
    builder.setPrivate(true);

    // Then: value is stored
    EXPECT_EQ(-1, builder.getMaxAgeSec());
    EXPECT_EQ(-1, builder.getMaxStaleSec());
    EXPECT_EQ(-1, builder.getMinFreshSec());
    EXPECT_EQ(-1, builder.getSMaxAgeSec());
    EXPECT_FALSE(builder.isMustRevalidate());
    EXPECT_FALSE(builder.isNoCache());
    EXPECT_FALSE(builder.isNoStore());
    EXPECT_FALSE(builder.isNoTransform());
    EXPECT_FALSE(builder.isOnlyIfCached());
    EXPECT_FALSE(builder.isPublic());
    EXPECT_TRUE(builder.isPrivate());
    CacheControl::Ptr pCacheControl = builder.build();
    EXPECT_EQ(-1, pCacheControl->getMaxAgeSec());
    EXPECT_EQ(-1, pCacheControl->getMaxStaleSec());
    EXPECT_EQ(-1, pCacheControl->getMinFreshSec());
    EXPECT_EQ(-1, pCacheControl->getSMaxAgeSec());
    EXPECT_FALSE(pCacheControl->isMustRevalidate());
    EXPECT_FALSE(pCacheControl->isNoCache());
    EXPECT_FALSE(pCacheControl->isNoStore());
    EXPECT_FALSE(pCacheControl->isNoTransform());
    EXPECT_FALSE(pCacheControl->isOnlyIfCached());
    EXPECT_FALSE(pCacheControl->isPublic());
    EXPECT_TRUE(pCacheControl->isPrivate());
    // TODO Fix expected value if toString() will be implement
    EXPECT_EQ("", pCacheControl->toString());
}

} /* namespace test */
} /* namespace easyhttpcpp */

