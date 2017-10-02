/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "easyhttpcpp/common/StringUtil.h"

#include "ConnectionInternal.h"
#include "KeepAliveTimeoutTask.h"
#include "MockConnectionPoolInternal.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

class ConnectionInternalSetInuseParameterUnitTest : public testing::Test {
};

class ConnectionReuseConditionParam {
public:
    const char* pConnectionUrl;
    const char* pConnectionProxyName;
    const unsigned short connectionProxyPort;
    const char* pConnectionRootCaDirectory;
    const char* pConnectionRootCaFile;
    const unsigned int connectionTimeoutSec;
    const char* pParameterUrl;
    const char* pParameterProxyName;
    const unsigned short parameterProxyPort;
    const char* pParameterRootCaDirectory;
    const char* pParameterRootCaFile;
    const unsigned int parameterTimeoutSec;
    bool retVal;
    ConnectionInternal::ConnectionStatus newStatus;
    bool isKeepAliveTimeoutTaskCancelled;
    bool isKeepAliveTimeoutTaskNull;

    std::string print() const
    {
        std::string ret = std::string("\n")
                + "connection url : " + (pConnectionUrl ? pConnectionUrl : "") + "\n"
                + "connection proxy name : " + (pConnectionProxyName ? pConnectionProxyName : "") + "\n"
                + "connection proxy port : " + StringUtil::format("%u", connectionProxyPort) + "\n"
                + "connection rootCa directory : " + (pConnectionRootCaDirectory ? pConnectionRootCaDirectory : "")
                + "\n"
                + "connection rootCa file : " + (pConnectionRootCaFile ? pConnectionRootCaFile : "") + "\n"
                + "connection timeout sec : " + StringUtil::format("%lu", connectionTimeoutSec) + "\n"
                + "parameter url : " + (pParameterUrl ? pParameterUrl : "") + "\n"
                + "parameter proxy name : " + (pParameterProxyName ? pParameterProxyName : "") + "\n"
                + "parameter proxy port : " + StringUtil::format("%u", parameterProxyPort) + "\n"
                + "parameter rootCa directory : " + (pParameterRootCaDirectory ? pParameterRootCaDirectory : "") + "\n"
                + "parameter rootCa file : " + (pParameterRootCaFile ? pParameterRootCaFile : "") + "\n"
                + "parameter timeout sec : " + StringUtil::format("%lu", parameterTimeoutSec) + "\n"
                + "retVal : " + StringUtil::boolToString(retVal) + "\n"
                + "new status : " + (ConnectionInternal::Idle == newStatus ? "Idle" : "Inuse") + "\n";
        return ret;
    }
};

static const ConnectionReuseConditionParam ConnectionReuseConditionData[] = {
    {   // 0: scheme == http, host, port, proxy, proxy port, timeout が同じ
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 1: host なし, scheme, port, proxy, proxy port, timeout が同じ
        "http://:9980/path",        // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://:9980/path",        // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 2: port なし, scheme, host, proxy, proxy port, timeout が同じ
        "http://host/path",         // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host/path",         // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 3: proxy なし, scheme, host, port, timeout が同じ
        "http://host:9980/path",    // connection url;
        NULL,                       // connection proxy name;
        0,                          // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        NULL,                       // parameter proxy name;
        0,                          // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 4: scheme == https, host, port, proxy, proxy port, rootCa directory, rootCa file, timeout が同じ
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        "rootCaDirectory",          // connection rootCa directory;
        "rootCaFile",               // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        "rootCaDirectory",          // parameter rootCa directory;
        "rootCaFile",               // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 5: scheme == https, rootCa directory なし, host, port, proxy, proxy port, rootCa file, timeout が同じ
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        "rootCaFile",               // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        "rootCaFile",               // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 6: scheme == https, rootCa file なし, host, port, proxy, proxy port, rootCa directory, timeout が同じ
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        "rootCaDirectory",          // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        "rootCaDirectory",          // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 7: scheme == http, rootCaDirectory が違う、host, port, proxy, proxy port, rootCa file, timeout が同じ
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        "rootCaDirectory",          // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        "rootCaDirectory2",         // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 8: scheme == http, rootCaFile が違う、host, port, proxy, proxy port, rootCa directory, timeout が同じ
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        "rootCaFile",               // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        "rootCaFile2",              // parameter rootCa file;
        10,                         // parameter timeout sec;
        true,                       // retVal;
        ConnectionInternal::Inuse   // new status
    },
    {   // 9: scheme が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 10: host が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host2:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 11: port が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:100/path",     // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 12: proxy が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy2",                   // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 13: proxy port が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9980,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 14: Connection に Proxy あり、parameter に Proxy なし
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        NULL,                       // parameter proxy name;
        0,                          // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 15: Connection に Proxy なし、parameter に Proxy あり
        "http://host:9980/path",    // connection url;
        NULL,                       // connection proxy name;
        0,                          // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 16: timeout が違う
        "http://host:9980/path",    // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "http://host:9980/path",    // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        20,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 17: scheme == https, rootCa directory が違う
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        "rootCaDirectory",          // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        "rootCaDirectory2",         // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 18: scheme == https, rootCa file が違う
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        "rootCaFile",               // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path",   // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        "rootCaFile2",              // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },
    {   // 19: parameter の url に decode できない文字が入っている。
        "https://host:9980/path",   // connection url;
        "proxy",                    // connection proxy name;
        9981,                       // connection proxy port;
        NULL,                       // connection rootCa directory;
        NULL,                       // connection rootCa file;
        10,                         // connection timeout sec;
        "https://host:9980/path%",  // parameter url;
        "proxy",                    // parameter proxy name;
        9981,                       // parameter proxy port;
        NULL,                       // parameter rootCa directory;
        NULL,                       // parameter rootCa file;
        10,                         // parameter timeout sec;
        false,                      // retVal;
        ConnectionInternal::Idle    // new status
    },

};

class ConnectionReuseConditionTest : public ConnectionInternalSetInuseParameterUnitTest,
        public testing::WithParamInterface<ConnectionReuseConditionParam> {
};
INSTANTIATE_TEST_CASE_P(ConnectionInternalSetInuseParameterUnitTest, ConnectionReuseConditionTest,
        testing::ValuesIn(ConnectionReuseConditionData));

// status == Idle で、Connection 再利用の組み合わせ確認。
// (scheme, host, port, proxy, proxy port, rootCa directory, rootCa file, timeout sec)
TEST_P(ConnectionReuseConditionTest,
        setInuseIfReusable_ReturnsTrueOrFalse_WhenStatusIsIdleAndByConnectionReuseCondition)
{
    ConnectionReuseConditionParam& param = (ConnectionReuseConditionParam&) GetParam();
    SCOPED_TRACE(param.print().c_str());

    // Given: setup ConnectionInternal and parameter.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    EasyHttpContext::Ptr pSourceEasyHttpContext = new EasyHttpContext();
    if (param.pConnectionProxyName != NULL) {
        Proxy::Ptr pSourceProxy = new Proxy(param.pConnectionProxyName, param.connectionProxyPort);
        pSourceEasyHttpContext->setProxy(pSourceProxy);
    }
    if (param.pConnectionRootCaDirectory != NULL) {
        pSourceEasyHttpContext->setRootCaDirectory(param.pConnectionRootCaDirectory);
    }
    if (param.pConnectionRootCaFile != NULL) {
        pSourceEasyHttpContext->setRootCaFile(param.pConnectionRootCaFile);
    }
    pSourceEasyHttpContext->setTimeoutSec(param.connectionTimeoutSec);
    ConnectionInternal::Ptr pConnectionInternal =
            new ConnectionInternal(pPocoHttpClientSession, param.pConnectionUrl, pSourceEasyHttpContext);

    // change status to Idle by onConnectionReleased.
    pConnectionInternal->onConnectionReleased();

    EasyHttpContext::Ptr pTargetEasyHttpContext = new EasyHttpContext();
    if (param.pParameterProxyName != NULL) {
        Proxy::Ptr pTargetProxy = new Proxy(param.pParameterProxyName, param.parameterProxyPort);
        pTargetEasyHttpContext->setProxy(pTargetProxy);
    }
    if (param.pParameterRootCaDirectory != NULL) {
        pTargetEasyHttpContext->setRootCaDirectory(param.pParameterRootCaDirectory);
    }
    if (param.pParameterRootCaFile != NULL) {
        pTargetEasyHttpContext->setRootCaFile(param.pParameterRootCaFile);
    }
    pTargetEasyHttpContext->setTimeoutSec(param.parameterTimeoutSec);

    // When: call setInuseIfReusable
    // Then: check return value, status.
    EXPECT_EQ(param.retVal, pConnectionInternal->setInuseIfReusable(param.pParameterUrl, pTargetEasyHttpContext));
    EXPECT_EQ(param.newStatus, pConnectionInternal->getStatus());
}

} /* namespace test */
} /* namespace easyhttpcpp */
