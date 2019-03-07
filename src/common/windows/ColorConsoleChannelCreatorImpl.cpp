/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/WindowsConsoleChannel.h"

#include "ColorConsoleChannelCreatorImpl.h"

namespace easyhttpcpp {
namespace common {

Poco::AutoPtr<Poco::Channel> ColorConsoleChannelCreatorImpl::create()
{
    return new Poco::WindowsColorConsoleChannel();
}

} /* namespace common */
} /* namespace easyhttpcpp */
