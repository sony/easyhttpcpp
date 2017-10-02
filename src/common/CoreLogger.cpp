/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"

namespace easyhttpcpp {
namespace common {

CoreLogger::CoreLogger()
{
}

CoreLogger::~CoreLogger()
{
}

CoreLogger& CoreLogger::getInstance()
{
    static Poco::SingletonHolder<CoreLogger> s_singleton;
    return *s_singleton.get();
}

} /* namespace common */
} /* namespace easyhttpcpp */
