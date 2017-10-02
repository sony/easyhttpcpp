/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

#include "ConnectionInternal.h"
#include "KeepAliveTimeoutListener.h"
#include "KeepAliveTimeoutTask.h"

namespace easyhttpcpp {

static const std::string Tag = "KeepAliveTimeoutTask";

KeepAliveTimeoutTask::KeepAliveTimeoutTask(Poco::Timestamp& expirationTime,
        KeepAliveTimeoutListener* pKeepAliveTimeoutListener)
        : m_keepAliveTimeoutExpirationTime(expirationTime), m_pKeepAliveTimeoutListener(pKeepAliveTimeoutListener)
{
    EASYHTTPCPP_LOG_D(Tag, "create this=[%p]", this);
}

KeepAliveTimeoutTask::~KeepAliveTimeoutTask()
{
}

void KeepAliveTimeoutTask::run()
{
    if (m_pKeepAliveTimeoutListener) {
        m_pKeepAliveTimeoutListener->onKeepAliveTimeoutExpired(this);
    }
}

Poco::Timestamp& KeepAliveTimeoutTask::getKeepAliveTimeoutExpirationTime()
{
    return m_keepAliveTimeoutExpirationTime;
}

} /* namespace easyhttpcpp */
