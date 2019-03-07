/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_ATOMICBOOL_H_INCLUDED
#define EASYHTTPCPP_COMMON_ATOMICBOOL_H_INCLUDED

#include "Poco/Mutex.h"

#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API AtomicBool {
public:
    AtomicBool(bool initValue);
    virtual ~AtomicBool();

    virtual void set(bool newValue);
    virtual bool get() const;

    /**
     * Atomically sets the value to the given updated value if the current value == the expected value.
     *
     * @param expect the expected value.
     * @param update the new value.
     * @return true if successful. False return indicates that the actual value was not equal to the expected value.
     */
    virtual bool compareAndSet(bool expect, bool update);

private:
    mutable Poco::FastMutex m_instanceMutex;
    bool m_value;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_ATOMICBOOL_H_INCLUDED */
