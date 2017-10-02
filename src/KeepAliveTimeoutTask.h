/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_KEEPALIVETIMEOUTTASK_H_INCLUDED
#define EASYHTTPCPP_KEEPALIVETIMEOUTTASK_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/TimerTask.h"

namespace easyhttpcpp {

class KeepAliveTimeoutListener;

class KeepAliveTimeoutTask : public Poco::Util::TimerTask {
public:
    typedef Poco::AutoPtr<KeepAliveTimeoutTask> Ptr;

    KeepAliveTimeoutTask(Poco::Timestamp& expirationTime, KeepAliveTimeoutListener* pKeepAliveTimeoutListener);
    virtual ~KeepAliveTimeoutTask();

    virtual void run();

    virtual Poco::Timestamp& getKeepAliveTimeoutExpirationTime();

private:
    Poco::Timestamp m_keepAliveTimeoutExpirationTime;
    KeepAliveTimeoutListener* m_pKeepAliveTimeoutListener;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_KEEPALIVETIMEOUTTASK_H_INCLUDED */
