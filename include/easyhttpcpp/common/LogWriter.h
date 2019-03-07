/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_LOGWRITER_H_INCLUDED
#define EASYHTTPCPP_COMMON_LOGWRITER_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/LogLevel.h"

namespace easyhttpcpp {
namespace common {

class LogWriter : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<LogWriter> Ptr;

    virtual ~LogWriter()
    {
    }

    virtual const std::string& getName() const = 0;
    virtual void log(const std::string& tag, LogLevel level, unsigned int line, const std::string& message) = 0;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_LOGWRITER_H_INCLUDED */
