/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Event.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_SYNCHRONIZEDEXECUTIONRUNNER_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_SYNCHRONIZEDEXECUTIONRUNNER_H_INCLUDED

namespace easyhttpcpp {
namespace test {

class SynchronizedExecutionRunner : public Poco::Runnable, public Poco::RefCountedObject {
public:
    SynchronizedExecutionRunner();
    virtual ~SynchronizedExecutionRunner();
    virtual void run();
    virtual bool execute() = 0;
    bool isSuccess();
    bool waitToReady();
    void setToStart();
    bool waitToFinish();
protected:
    void setToReady();
    bool waitToStart();
    void setToFinish();
private:
    bool m_succeeded;
    Poco::Event m_readyEvent;
    Poco::Event m_startEvent;
    Poco::Event m_finishEvent;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_SYNCHRONIZEDEXECUTIONRUNNER_H_INCLUDED */
