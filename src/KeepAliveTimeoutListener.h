/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_KEEPALIVETIMEOUTLISTENER_H_INCLUDED
#define EASYHTTPCPP_KEEPALIVETIMEOUTLISTENER_H_INCLUDED

namespace easyhttpcpp {

class KeepAliveTimeoutTask;

class KeepAliveTimeoutListener {
public:
    virtual ~KeepAliveTimeoutListener()
    {
    }

    virtual void onKeepAliveTimeoutExpired(const KeepAliveTimeoutTask* pKeepAliveTimeoutTask) = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_KEEPALIVETIMEOUTLISTENER_H_INCLUDED */
