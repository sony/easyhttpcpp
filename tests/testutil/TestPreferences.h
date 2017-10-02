/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TESTPREFERENCES_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TESTPREFERENCES_H_INCLUDED

#include <string>

#include "Poco/SingletonHolder.h"
#include "Poco/Path.h"
#include "Poco/JSON/Object.h"

#include "easyhttpcpp/Proxy.h"

using easyhttpcpp::Proxy;

namespace easyhttpcpp {
namespace testutil {

class TestPreferences {
public:
    enum ProfileType {
        ProfileQA,
    };
    
    static TestPreferences& getInstance();
    
    virtual void initialize(ProfileType prefsProfile);
    virtual Proxy::Ptr optGetProxy(Proxy::Ptr pDefaultValue);
    virtual Poco::SharedPtr<Poco::Path> optGetRootCaDirPath(Poco::Path* pDefaultValue);
    virtual Poco::SharedPtr<Poco::Path> optGetRootCaFilePath(Poco::Path* pDefaultValue);
    
private:
    TestPreferences();
    virtual ~TestPreferences();
    
    bool m_initialized;
    
    friend class Poco::SingletonHolder<TestPreferences>;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TESTPREFERENCES_H_INCLUDED */
