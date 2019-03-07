/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/AutoPtr.h"

#include "ColorConsoleChannelCreator.h"
#include "ColorConsoleChannelCreatorImpl.h"

namespace easyhttpcpp {
namespace common {

Poco::AutoPtr<Poco::Channel> ColorConsoleChannelCreator::create()
{
    return ColorConsoleChannelCreatorImpl::create();
}

} /* namespace common */
} /* namespace easyhttpcpp */
