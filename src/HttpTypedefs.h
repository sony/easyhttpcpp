/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPTYPEDEFS_H_INCLUDED
#define EASYHTTPCPP_HTTPTYPEDEFS_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Net/HTTPClientSession.h"

namespace easyhttpcpp {

typedef Poco::SharedPtr<Poco::Net::HTTPClientSession> PocoHttpClientSessionPtr;

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPTYPEDEFS_H_INCLUDED */
