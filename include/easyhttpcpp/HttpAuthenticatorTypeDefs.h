/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPAUTHENTICATORTYPEDEFS_H_INCLUDED
#define EASYHTTPCPP_HTTPAUTHENTICATORTYPEDEFS_H_INCLUDED

#include <string>

#include "Poco/SharedPtr.h"

#include "easyhttpcpp/common/Future.h"

namespace easyhttpcpp {

typedef Poco::SharedPtr<easyhttpcpp::common::Future<std::string> > HttpAuthorizationFuturePtr;

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPAUTHENTICATORTYPEDEFS_H_INCLUDED */
