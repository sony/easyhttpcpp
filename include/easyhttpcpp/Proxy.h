/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_PROXY_H_INCLUDED
#define EASYHTTPCPP_PROXY_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

namespace easyhttpcpp {

/**
 * @brief A Proxy preserve proxy.
 */
class Proxy : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Proxy> Ptr;

    /**
     * 
     * @param host proxy host name
     * @param port proxy port
     */
    Proxy(const std::string& host, unsigned short port);

    /**
     * 
     */
    virtual ~Proxy();

    /**
     * @brief Get host name
     * @return host name
     */
    virtual const std::string& getHost() const;

    /**
     * @brief Get port
     * @return port
     */
    virtual unsigned short getPort() const;

    virtual std::string toString();

    /**
     * @brief Compare proxy
     * @param proxy proxy
     * @return true if both proxy are identical, false otherwise.
     */
    virtual bool operator==(const Proxy& proxy) const;

    /**
     * @brief Compare proxy
     * @param proxy proxy
     * @return false if both proxy are identical, true otherwise.
     */
    virtual bool operator!=(const Proxy& proxy) const;

private:
    bool equals(const Proxy& proxy) const;

    std::string m_host;
    unsigned short m_port;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_PROXY_H_INCLUDED */
