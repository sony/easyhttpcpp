/*
 * Copyright 2018 Sony Corporation
 */

#ifndef EASYHTTPCPP_REQUESTBODYUTIL_H_INCLUDED
#define EASYHTTPCPP_REQUESTBODYUTIL_H_INCLUDED

#include <istream>
#include <ostream>
#include <string>

#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

#include "easyhttpcpp/common/ByteArrayBuffer.h"

namespace easyhttpcpp {

class RequestBodyUtil {
public:
    static void write(const easyhttpcpp::common::ByteArrayBuffer& inBuffer, std::ostream& outStream);
    static void write(std::istream& inStream, std::ostream& outStream);
    static void write(const std::string& inBuffer, std::ostream& outStream);

private:
    RequestBodyUtil();
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_REQUESTBODYUTIL_H_INCLUDED */
