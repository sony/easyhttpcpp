/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/AtomicBool.h"

namespace easyhttpcpp {
namespace common {

AtomicBool::AtomicBool(bool initValue) : m_value(initValue)
{
}

AtomicBool::~AtomicBool()
{
}

void AtomicBool::set(bool newValue)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    m_value = newValue;
}

bool AtomicBool::get() const
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    return m_value;
}

bool AtomicBool::compareAndSet(bool expect, bool update)
{
    Poco::FastMutex::ScopedLock lock(m_instanceMutex);

    if (m_value != expect) {
        return false;
    }

    m_value = update;
    return true;
}

} /* namespace common */
} /* namespace easyhttpcpp */
