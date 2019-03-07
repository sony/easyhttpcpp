/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_EXECUTORSERVICE_BLOCKINGQUEUE_H_INCLUDED
#define EASYHTTPCPP_EXECUTORSERVICE_BLOCKINGQUEUE_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/RefCountedRunnable.h"

namespace easyhttpcpp {
namespace executorservice {

class BlockingQueue : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<BlockingQueue> Ptr;

    virtual ~BlockingQueue()
    {
    }

    virtual bool push(easyhttpcpp::common::RefCountedRunnable::Ptr pTask) = 0;
    virtual easyhttpcpp::common::RefCountedRunnable::Ptr pop() = 0;
    virtual bool isEmpty() = 0;
    virtual void clear() = 0;
protected:
    Poco::FastMutex m_mutex;
};

} /* namespace executorservice */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_EXECUTORSERVICE_BLOCKINGQUEUE_H_INCLUDED */
