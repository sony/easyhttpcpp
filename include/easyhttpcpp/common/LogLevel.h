/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_LOGLEVEL_H_INCLUDED
#define EASYHTTPCPP_COMMON_LOGLEVEL_H_INCLUDED

namespace easyhttpcpp {
namespace common {

enum LogLevel {
    LogLevelVerbose,
    LogLevelDebug,
    LogLevelInfo,
    LogLevelWarning,
    LogLevelError,
    LogLevelSilent
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_LOGLEVEL_H_INCLUDED */
