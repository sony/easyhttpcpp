/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATOR_H_INCLUDED
#define EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATOR_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"

namespace easyhttpcpp {
namespace common {

class ColorConsoleChannelCreator {
public:
    static Poco::AutoPtr<Poco::Channel> create();
private:
    ColorConsoleChannelCreator();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COLORCONSOLECHANNELCREATOR_H_INCLUDED */
