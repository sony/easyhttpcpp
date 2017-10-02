/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPINTEGRATIONTESTCASE_H_INCLUDED
#define EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPINTEGRATIONTESTCASE_H_INCLUDED

#include "gtest/gtest.h"

#include "Poco/Crypto/OpenSSLInitializer.h"
#include "Poco/Path.h"

#include "easyhttpcpp/EasyHttp.h"
#include "TestPreferences.h"

using easyhttpcpp::testutil::TestPreferences;

namespace easyhttpcpp {
namespace test {

class HttpIntegrationTestCase : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // initialize test preferences with QA profile
        TestPreferences::getInstance().initialize(TestPreferences::ProfileQA);
    }

    virtual ~HttpIntegrationTestCase()
    {
    }
    
    void setEasyHttpConfigFromTestPreferences(EasyHttp::Builder& builder)
    {
        builder.setProxy(TestPreferences::getInstance().optGetProxy(NULL));
        Poco::SharedPtr<Poco::Path> pRootCaDirPath = TestPreferences::getInstance().optGetRootCaDirPath(NULL);
        if (pRootCaDirPath) {
            builder.setRootCaDirectory(pRootCaDirPath->toString());
        }
        Poco::SharedPtr<Poco::Path> pRootCaFilePath = TestPreferences::getInstance().optGetRootCaFilePath(NULL);
        if (pRootCaFilePath) {
            builder.setRootCaFile(pRootCaFilePath->toString());
        }
    }
    
protected:
    Poco::Crypto::OpenSSLInitializer m_openSslInitializer;
};

} /* namespace test */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TEST_INTEGRATIONTEST_HTTPINTEGRATIONTESTCASE_H_INCLUDED */
