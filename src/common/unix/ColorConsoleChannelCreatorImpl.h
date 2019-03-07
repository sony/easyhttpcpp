/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATORIMPL_H_INCLUDED
#define EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATORIMPL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"

namespace easyhttpcpp {
namespace common {

class ColorConsoleChannelCreatorImpl {
public:
    static Poco::AutoPtr<Poco::Channel> create();
private:
    ColorConsoleChannelCreatorImpl();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATORIMPL_H_INCLUDED */
