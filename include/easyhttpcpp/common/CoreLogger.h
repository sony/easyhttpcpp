/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CORELOGGER_H_INCLUDED
#define EASYHTTPCPP_COMMON_CORELOGGER_H_INCLUDED

#include "Poco/SingletonHolder.h"

#include "easyhttpcpp/common/BaseLogger.h"
#include "easyhttpcpp/common/LogLevel.h"
#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace common {

#define EASYHTTPCPP_LOG_E(pTag, pFormat, ...) EASYHTTPCPP_BASE_LOG_E(easyhttpcpp::common::CoreLogger::getInstance(), pTag, \
        pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_LOG_W(pTag, pFormat, ...) EASYHTTPCPP_BASE_LOG_W(easyhttpcpp::common::CoreLogger::getInstance(), pTag, \
        pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_LOG_I(pTag, pFormat, ...) EASYHTTPCPP_BASE_LOG_I(easyhttpcpp::common::CoreLogger::getInstance(), pTag, \
        pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_LOG_D(pTag, pFormat, ...) EASYHTTPCPP_BASE_LOG_D(easyhttpcpp::common::CoreLogger::getInstance(), pTag, \
        pFormat, ##__VA_ARGS__)
#define EASYHTTPCPP_LOG_V(pTag, pFormat, ...) EASYHTTPCPP_BASE_LOG_V(easyhttpcpp::common::CoreLogger::getInstance(), pTag, \
        pFormat, ##__VA_ARGS__)

class CoreLogger : public BaseLogger {
public:
    static CoreLogger& getInstance();

private:
    CoreLogger();
    virtual ~CoreLogger();

    friend class Poco::SingletonHolder<CoreLogger>;

};
} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CORELOGGER_H_INCLUDED */
