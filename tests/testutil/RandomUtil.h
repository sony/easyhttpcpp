/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_RANDOMUTIL_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_RANDOMUTIL_H_INCLUDED

namespace easyhttpcpp {
namespace testutil {

class RandomUtil {
public:
    static void getRandomBytes(size_t size, easyhttpcpp::common::ByteArrayBuffer& buffer);

private:
    RandomUtil();

};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_RANDOMUTIL_H_INCLUDED */
