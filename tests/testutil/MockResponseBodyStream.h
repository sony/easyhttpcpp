/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_MOCKRESPONSEBODYSTREAM_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_MOCKRESPONSEBODYSTREAM_H_INCLUDED

#include "easyhttpcpp/ResponseBodyStream.h"

namespace easyhttpcpp {
namespace testutil {

class MockResponseBodyStream : public easyhttpcpp::ResponseBodyStream {
public:
    MOCK_METHOD2(read, ssize_t(char* pBuffer, size_t readBytes));
    MOCK_METHOD0(isEof, bool());
    MOCK_METHOD0(close, void());
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_MOCKRESPONSEBODYSTREAM_H_INCLUDED */
