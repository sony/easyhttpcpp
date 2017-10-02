/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Random.h"
#include "Poco/Buffer.h"

#include "easyhttpcpp/common/ByteArrayBuffer.h"
#include "easyhttpcpp/common/Typedef.h"
#include "RandomUtil.h"

using easyhttpcpp::common::Byte;
using easyhttpcpp::common::ByteArrayBuffer;

namespace easyhttpcpp {
namespace testutil {

void RandomUtil::getRandomBytes(size_t size, ByteArrayBuffer& buffer)
{
    Poco::Random random;
    Poco::Buffer<Byte> bytes(size);
    for (size_t i = 0; i < size; i++) {
        bytes[i] = random.nextChar();
    }
    buffer.write(bytes.begin(), bytes.size());
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
