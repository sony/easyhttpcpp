/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/BaseException.h"
#include "easyhttpcpp/executorservice/Task.h"

using easyhttpcpp::common::BaseException;
using easyhttpcpp::common::CoreLogger;
using easyhttpcpp::common::RefCountedRunnable;

namespace easyhttpcpp {
namespace executorservice {

static const std::string Tag = "Task";

Task::Task() : m_cancelled(false), m_finished(false)
{
}

Task::~Task()
{
}

void Task::run()
{
    RefCountedRunnable::Ptr guard(this, false); // ensure automatic release when done
    try {
        runTask();
    } catch (const BaseException& e) {
        EASYHTTPCPP_LOG_V(Tag, "Task threw exception: %s", e.getMessage().c_str());
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_V(Tag, "Task threw poco exception: %s", e.message().c_str());
    } catch (const std::exception& e) {
        EASYHTTPCPP_LOG_V(Tag, "Task threw std exception: %s", e.what());
    } catch (...) {
        EASYHTTPCPP_LOG_V(Tag, "Task threw unknown exception.");
    }

    m_finished.set(true);
}

bool Task::cancel(bool mayInterruptIfRunning)
{
    // classes overriding may use mayInterruptIfRunning; base implementation does nothing

    m_cancelled.set(true);
    return true;
}

bool Task::isCancelled() const
{
    return m_cancelled.get();
}

bool Task::isDone() const
{
    return m_finished.get();
}

} /* namespace executorservice */
} /* namespace easyhttpcpp */

