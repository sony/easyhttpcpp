/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/Proxy.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

Proxy::Proxy(const std::string& host, unsigned short port) : m_host(host), m_port(port)
{
}

Proxy::~Proxy()
{
}

const std::string& Proxy::getHost() const
{
    return m_host;
}

unsigned short Proxy::getPort() const
{
    return m_port;
}

std::string Proxy::toString()
{
    return StringUtil::format("%s:%d", m_host.c_str(), m_port);
}

bool Proxy::operator == (const Proxy& proxy) const
{
    return equals(proxy);
}

bool Proxy::operator != (const Proxy& proxy) const
{
    return !equals(proxy);
}

bool Proxy::equals(const Proxy& proxy) const
{
    return (m_host == proxy.m_host && m_port == proxy.m_port);
}

} /* namespace easyhttpcpp */
