/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

namespace easyhttpcpp {
namespace common {

bool CoreLogger::s_terminated = false;

CoreLogger::CoreLogger()
{
}

CoreLogger::~CoreLogger()
{
    s_terminated = true;
}

CoreLogger* CoreLogger::getInstance()
{
    static Poco::SingletonHolder<CoreLogger> s_singleton;

    if (s_terminated) {
        return NULL;
    } else {
        return s_singleton.get();
    }
}

} /* namespace common */
} /* namespace easyhttpcpp */
