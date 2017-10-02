/*
 * Copyright 2017 Sony Corporation
 */

#include <stdexcept>
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/Proxy.h"
#include "TestPreferences.h"

using easyhttpcpp::Proxy;

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestPreferences";

TestPreferences::TestPreferences() : m_initialized(false)
{
}

TestPreferences::~TestPreferences()
{
}

TestPreferences& TestPreferences::getInstance()
{
    static Poco::SingletonHolder<TestPreferences> s_singleton;
    return *s_singleton.get();
}

void TestPreferences::initialize(ProfileType prefsProfile)
{
    m_initialized = true;
}

Proxy::Ptr TestPreferences::optGetProxy(Proxy::Ptr pDefaultValue)
{
    if (!m_initialized) {
        throw std::runtime_error("Test preferences not yet initialized.");
    }
    
    return pDefaultValue;
}

Poco::SharedPtr<Poco::Path> TestPreferences::optGetRootCaDirPath(Poco::Path* pDefaultValue)
{
    if (!m_initialized) {
        throw std::runtime_error("Test preferences not yet initialized.");
    }
    
    return pDefaultValue;    
}

Poco::SharedPtr<Poco::Path> TestPreferences::optGetRootCaFilePath(Poco::Path* pDefaultValue)
{
    if (!m_initialized) {
        throw std::runtime_error("Test preferences not yet initialized.");
    }
    
    return pDefaultValue;    
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
