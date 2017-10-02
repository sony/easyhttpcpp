/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKREQUEST_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKREQUEST_H_INCLUDED

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "easyhttpcpp/Request.h"

namespace easyhttpcpp {
namespace testutil {

class MockRequest : public easyhttpcpp::Request {
public:
    MockRequest(easyhttpcpp::Request::Builder& builder) : easyhttpcpp::Request(builder)
    {
    }

    MOCK_CONST_METHOD0(getBody, easyhttpcpp::RequestBody::Ptr());
    MOCK_CONST_METHOD0(getCacheControl, easyhttpcpp::CacheControl::Ptr());
    MOCK_CONST_METHOD2(getHeaderValue, const std::string&(const std::string& name, const std::string& defaultValue));
    MOCK_CONST_METHOD1(hasHeader, bool(const std::string& name));
    MOCK_CONST_METHOD0(getHeaders, easyhttpcpp::Headers::Ptr());
    MOCK_CONST_METHOD0(getMethod, easyhttpcpp::Request::HttpMethod());
    MOCK_CONST_METHOD0(getTag, const void*());
    MOCK_CONST_METHOD0(getUrl, const std::string&());
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKREQUEST_H_INCLUDED */
