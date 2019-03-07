/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/ConsoleChannel.h"

#include "ColorConsoleChannelCreatorImpl.h"

namespace easyhttpcpp {
namespace common {

Poco::AutoPtr<Poco::Channel> ColorConsoleChannelCreatorImpl::create()
{
    return new Poco::ColorConsoleChannel();
}

} /* namespace common */
} /* namespace easyhttpcpp */
