/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CONNECTIONSTATUSLISTENER_H_INCLUDED
#define EASYHTTPCPP_CONNECTIONSTATUSLISTENER_H_INCLUDED

namespace easyhttpcpp {

class ConnectionInternal;

class ConnectionStatusListener {
public:
    virtual ~ConnectionStatusListener()
    {
    }

    virtual void onIdle(ConnectionInternal* pConnectionInternal, bool& listenerInvalidated) = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CONNECTIONSTATUSLISTENER_H_INCLUDED */
