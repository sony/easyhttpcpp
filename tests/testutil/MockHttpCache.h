/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKHTTPCACHE_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKHTTPCACHE_H_INCLUDED

#include "gmock/gmock.h"

#include "easyhttpcpp/HttpCache.h"

namespace easyhttpcpp {
namespace testutil {

class MockHttpCache : public easyhttpcpp::HttpCache {
public:
    typedef Poco::AutoPtr<MockHttpCache> Ptr;

    virtual ~MockHttpCache()
    {
    }

    MOCK_CONST_METHOD0(getPath, const Poco::Path&());
    MOCK_METHOD0(evictAll, void());
    MOCK_CONST_METHOD0(getMaxSize, size_t());
    MOCK_METHOD0(getSize, size_t());
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKHTTPCACHE_H_INCLUDED */
