/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKCACHE_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKCACHE_H_INCLUDED

#include "gmock/gmock.h"

#include "easyhttpcpp/common/Cache.h"

namespace easyhttpcpp {
namespace testutil {

class MockCache : public easyhttpcpp::common::Cache {
public:
    MOCK_METHOD2(getMetadata, bool(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr& pCacheMetadata));
    MOCK_METHOD2(getData, bool(const std::string& key, std::istream*& pStream));
    MOCK_METHOD3(get, bool(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr& pCacheMetadata,
            std::istream*& pStream));
    MOCK_METHOD2(putMetadata, bool(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata));
    MOCK_METHOD3(put, bool(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata,
            const std::string& path));
    MOCK_METHOD3(put, bool(const std::string& key, easyhttpcpp::common::CacheMetadata::Ptr pCacheMetadata,
            Poco::SharedPtr<easyhttpcpp::common::ByteArrayBuffer> pData));
    MOCK_METHOD1(remove, bool(const std::string& key));
    MOCK_METHOD1(releaseData, void(const std::string& key));
    MOCK_METHOD1(purge, bool(bool mayDeleteIfBusy));
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKCACHE_H_INCLUDED */
