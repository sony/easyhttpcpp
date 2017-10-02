/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CONNECTION_H_INCLUDED
#define EASYHTTPCPP_CONNECTION_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

namespace easyhttpcpp {

/**
 * @brief A Connection store http connection information.
 */
class Connection : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Connection> Ptr;

    virtual ~Connection()
    {
    }

    /**
     * @brief Get http protocol (ex. "HTTP/1.1")
     * @return protocol string.
     */
    virtual const std::string& getProtocol() const = 0;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CONNECTION_H_INCLUDED */
