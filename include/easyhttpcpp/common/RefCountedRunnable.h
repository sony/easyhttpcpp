/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_REFCOUNTEDRUNNABLE_H_INCLUDED
#define EASYHTTPCPP_COMMON_REFCOUNTEDRUNNABLE_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"

namespace easyhttpcpp {
namespace common {

class RefCountedRunnable : public Poco::Runnable, public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<RefCountedRunnable> Ptr;

};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_REFCOUNTEDRUNNABLE_H_INCLUDED */
