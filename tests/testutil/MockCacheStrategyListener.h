/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKCACHESTRATEGYLISTENER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKCACHESTRATEGYLISTENER_H_INCLUDED

#include "easyhttpcpp/common/CacheInfoWithDataSize.h"
#include "easyhttpcpp/common/CacheStrategyListener.h"

namespace easyhttpcpp {
namespace testutil {

class MockCacheStrategyListener :
public easyhttpcpp::common::CacheStrategyListener<std::string, easyhttpcpp::common::CacheInfoWithDataSize::Ptr> {
public:
    MOCK_METHOD2(onAdd, bool(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value));
    MOCK_METHOD2(onUpdate, bool(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value));
    MOCK_METHOD1(onRemove, bool(const std::string& key));
    MOCK_METHOD2(onGet, bool(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr value));
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKCACHESTRATEGYLISTENER_H_INCLUDED */
