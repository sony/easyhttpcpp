/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_MOCKHTTPCACHEENUMERATIONLISTENER_H_INCLUDED
#define EASYHTTPCPP_TEST_MOCKHTTPCACHEENUMERATIONLISTENER_H_INCLUDED

#include "gmock/gmock.h"

#include "MockHttpCacheEnumerationListener.h"

namespace easyhttpcpp {
namespace test {

class MockHttpCacheEnumerationListener : public HttpCacheEnumerationListener {
public:
    MOCK_METHOD1(onEnumerate, bool(const EnumerationParam& param));
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_MOCKHTTPCACHEENUMERATIONLISTENER_H_INCLUDED */
