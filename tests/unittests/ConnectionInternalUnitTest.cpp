/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/Net/HTTPMessage.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"

#include "ConnectionInternal.h"
#include "KeepAliveTimeoutTask.h"
#include "MockConnectionPoolInternal.h"
#include "MockConnectionStatusListener.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace test {

static const char* const SchemeHttp = "http";
static const char* const HostName = "host";
static const unsigned short HostPort = 9982;
static const char* const ProxyName = "proxy";
static const unsigned short ProxyPort = 1080;
static const char* const RootCaDirectory = "rootCaDirectory";
static const char* const RootCaFile = "RootCaFile";
static const unsigned int TimeoutSec = 10;
static const char* const TestDefaultUrl = "http://host:9980/path";

class ConnectionInternalUnitTest : public testing::Test {
};

// parameter で指定された情報が設定される。
//
// 1. PocoHttpClientSession が設定される。
// 2. url から、scheme, host, port が設定される。
// 3. EasyHttpContext から、proxy、rootCaDirectory, rootCaFile, timeOutSec が設定される。
TEST_F(ConnectionInternalUnitTest, constructor_SetsSpecifiedParameter_WhenWithParameter)
{
    // Given: prepare parameter for ConnectionInternal.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = StringUtil::format("%s://%s:%u/path", SchemeHttp, HostName, HostPort);
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setProxy(pProxy);
    pEasyHttpContext->setRootCaDirectory(RootCaDirectory);
    pEasyHttpContext->setRootCaFile(RootCaFile);
    pEasyHttpContext->setTimeoutSec(TimeoutSec);

    // When: create ConnectionInternal.
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Then: set specified parameters.
    ASSERT_FALSE(pConnectionInternal.isNull());
    EXPECT_EQ(pPocoHttpClientSession, pConnectionInternal->getPocoHttpClientSession());
    EXPECT_EQ(SchemeHttp, pConnectionInternal->getScheme());
    EXPECT_EQ(HostName, pConnectionInternal->getHostName());
    EXPECT_EQ(HostPort, pConnectionInternal->getHostPort());
    EXPECT_EQ(ProxyName, pConnectionInternal->getProxy()->getHost());
    EXPECT_EQ(ProxyPort, pConnectionInternal->getProxy()->getPort());
    EXPECT_EQ(RootCaDirectory, pConnectionInternal->getRootCaDirectory());
    EXPECT_EQ(RootCaFile, pConnectionInternal->getRootCaFile());
    EXPECT_EQ(TimeoutSec, pConnectionInternal->getTimeoutSec());
}

// parameter で指定された情報が設定される。(urlにhost なし)
//
// host が空文字列として設定される。
TEST_F(ConnectionInternalUnitTest, constructor_SetsSpecifiedParameterAndHostNameIsEmpty_WhenNoHostNameInUrl)
{
    // Given: no host name in url.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = StringUtil::format("%s://:%u/path", SchemeHttp, HostPort);
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setProxy(pProxy);
    pEasyHttpContext->setRootCaDirectory(RootCaDirectory);
    pEasyHttpContext->setRootCaFile(RootCaFile);
    pEasyHttpContext->setTimeoutSec(TimeoutSec);

    // When: create ConnectionInternal.
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Then: host name is empty string and other parameters were set.
    ASSERT_FALSE(pConnectionInternal.isNull());
    EXPECT_EQ(pPocoHttpClientSession, pConnectionInternal->getPocoHttpClientSession());
    EXPECT_EQ(SchemeHttp, pConnectionInternal->getScheme());
    EXPECT_TRUE(pConnectionInternal->getHostName().empty());
    EXPECT_EQ(HostPort, pConnectionInternal->getHostPort());
    EXPECT_EQ(ProxyName, pConnectionInternal->getProxy()->getHost());
    EXPECT_EQ(ProxyPort, pConnectionInternal->getProxy()->getPort());
    EXPECT_EQ(RootCaDirectory, pConnectionInternal->getRootCaDirectory());
    EXPECT_EQ(RootCaFile, pConnectionInternal->getRootCaFile());
    EXPECT_EQ(TimeoutSec, pConnectionInternal->getTimeoutSec());
}

// parameter で指定された情報が設定される。(urlにportなし)
//
// port = 80 (Poco の default値)で設定される。
TEST_F(ConnectionInternalUnitTest, constructor_SetsSpecifiedParameterAndHostPortIsDefaultValue_WhenNoHostPortNameInUrl)
{
    // Given: no port in url.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = StringUtil::format("%s://%s/path", SchemeHttp, HostName);
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setProxy(pProxy);
    pEasyHttpContext->setRootCaDirectory(RootCaDirectory);
    pEasyHttpContext->setRootCaFile(RootCaFile);
    pEasyHttpContext->setTimeoutSec(TimeoutSec);

    // When: create ConnectionInernal.
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Then: port is 80 and other parameters were set.
    ASSERT_FALSE(pConnectionInternal.isNull());
    EXPECT_EQ(pPocoHttpClientSession, pConnectionInternal->getPocoHttpClientSession());
    EXPECT_EQ(SchemeHttp, pConnectionInternal->getScheme());
    EXPECT_EQ(HostName, pConnectionInternal->getHostName());
    EXPECT_EQ(80, pConnectionInternal->getHostPort());
    EXPECT_EQ(ProxyName, pConnectionInternal->getProxy()->getHost());
    EXPECT_EQ(ProxyPort, pConnectionInternal->getProxy()->getPort());
    EXPECT_EQ(RootCaDirectory, pConnectionInternal->getRootCaDirectory());
    EXPECT_EQ(RootCaFile, pConnectionInternal->getRootCaFile());
    EXPECT_EQ(TimeoutSec, pConnectionInternal->getTimeoutSec());
}

// parameter で指定された情報が設定される。(proxy=NULL)
//
// proxy が NULL で設定される。
TEST_F(ConnectionInternalUnitTest, constructor_SetsSpecifiedParameter_WhenWithoutProxy)
{
    // Given: not set Proxy to EasyHttpContext.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = StringUtil::format("%s://%s:%u/path", SchemeHttp, HostName, HostPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setRootCaDirectory(RootCaDirectory);
    pEasyHttpContext->setRootCaFile(RootCaFile);
    pEasyHttpContext->setTimeoutSec(TimeoutSec);

    // When: create ConnectionInternal.
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Then: Proxt is NULL and other parameters were set.
    ASSERT_FALSE(pConnectionInternal.isNull());
    EXPECT_EQ(pPocoHttpClientSession, pConnectionInternal->getPocoHttpClientSession());
    EXPECT_EQ(SchemeHttp, pConnectionInternal->getScheme());
    EXPECT_EQ(HostName, pConnectionInternal->getHostName());
    EXPECT_EQ(HostPort, pConnectionInternal->getHostPort());
    EXPECT_TRUE(pConnectionInternal->getProxy().isNull());
    EXPECT_EQ(RootCaDirectory, pConnectionInternal->getRootCaDirectory());
    EXPECT_EQ(RootCaFile, pConnectionInternal->getRootCaFile());
    EXPECT_EQ(TimeoutSec, pConnectionInternal->getTimeoutSec());
}

// url がdecode できない文字列の場合。
//
// HttpExecutionExecption が throw される。
TEST_F(ConnectionInternalUnitTest, constructor_ThrowHttpExecutionException_WhenUndecodableUrl)
{
    // Given: undecodable url string.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = "http://host/path%";
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    pEasyHttpContext->setRootCaDirectory(RootCaDirectory);
    pEasyHttpContext->setRootCaFile(RootCaFile);
    pEasyHttpContext->setTimeoutSec(TimeoutSec);

    // When: create ConnectionInternal.
    // Then: throw HttpExecutionException.
    EASYHTTPCPP_EXPECT_THROW_WITH_CAUSE(new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext),
            HttpExecutionException, 100702);
}

TEST_F(ConnectionInternalUnitTest, getProtocol_ReturnsHttpOnePointOne_WhenAlways)
{
    // Given: create ConnectionInternal by any parameters.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // When: call getPtotocol().
    // Then: return "HTTP/1.1".
    EXPECT_EQ(Poco::Net::HTTPMessage::HTTP_1_1, pConnectionInternal->getProtocol());
}

// statusを切り替えて、getStatus を呼び出す。
//
// status == Inuseの時、Inuse が返る。
// status == Idle の時、Idle が返る。
TEST_F(ConnectionInternalUnitTest, getStatus_ReturnsCurrentStatus_WhenInuseOrIdle)
{
    // Given: create ConnectionInternal by any parameters.
    // status is Inuse
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // When: call getStatus
    // Then: return Inuse
    EXPECT_EQ(ConnectionInternal::Inuse, pConnectionInternal->getStatus());

    // change status to Idle by onConnectionReleased.
    pConnectionInternal->onConnectionReleased();

    // return Idle
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
}

// status == Inuse での呼び出し。
//
// false が返る。
TEST_F(ConnectionInternalUnitTest, setInuseIfReusable_ReturnsFalse_WhenStatusIsInuse)
{
    // Given: create ConnectionInternal by any parameters.
    // status is Inuse
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // When: call setInuseIfReusable
    // Then: return false
    EXPECT_FALSE(pConnectionInternal->setInuseIfReusable(url, pEasyHttpContext));
}

// 1. status == Inuse
// 2. setConnectionStateListener が登録されてる。
// 3. cancel されていない。
// 4. ConnectinStatusListener::onIdle で listenerInvalidated=true にする。
// 確認内容
// 1. true が返る。
// 2. ConnectinStatusListener::onIdle が呼び出され、listenerInvalidated=true にする。
// 3. status が Idle になる。
// 4. ConnectionPoolInternal::startKeepAliveTimer が呼び出される。
// 5. ConnectionStatusListener の参照が削除される。
TEST_F(ConnectionInternalUnitTest,
        onConnectionRelease_ReturnsTrueAndReleaseReferenceOfConnectionStatusListener_WhenStatusIsInuseAndSetConnectionStatusListenerAndListenerInvalidatedIsTrueByOnIdle)
{
    // Given: status is Inuse, set ConnectionStatusListener.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    MockConnectionStatusListener mockConnectionStatusListener;
    EXPECT_CALL(mockConnectionStatusListener, onIdle(pConnectionInternal.get(), testing::_))
            .WillOnce(testing::SetArgReferee<1>(true));
    pConnectionInternal->setConnectionStatusListener(&mockConnectionStatusListener);

    // When: call onConnectionReleased, listenerInvalidated is true by ConnectionStatusListener::onIdle.
    // Then: return true, call ConnectionStatusListener, status is Idle, remove ConnectionStatusListener.
    pConnectionInternal->onConnectionReleased();

    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    EXPECT_TRUE(pConnectionInternal->getConnectionStatusListener() == NULL);
}

// 1. status == Inuse
// 2. setConnectionStateListener が登録されてる。
// 3. ConnectinStatusListener::onIdle で listenerInvalidated=false にする。
// 確認内容
// 1. true が返る。
// 2. ConnectinStatusListener::onIdle が呼び出され、listenerInvalidated=false にする。
// 3. status が Idle になる。
// 4. ConnectionPoolInternal::startKeepAliveTimer が呼び出される。
// 5. ConnectionStatusListener の参照は削除されない。
TEST_F(ConnectionInternalUnitTest,
        onConnectionReleased_ReturnsTrueAndNotReleaseReferenceOfConnectionStatusListener_WhenStatusIsInuseAndSetConnectionStatusListenerAndListenerInvalidatedIsFalseByOnIdle)
{
    // Given: status is Inuse, set ConnectionStatusListener.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    MockConnectionStatusListener mockConnectionStatusListener;
    EXPECT_CALL(mockConnectionStatusListener, onIdle(pConnectionInternal.get(), testing::_))
            .WillOnce(testing::SetArgReferee<1>(false));
    pConnectionInternal->setConnectionStatusListener(&mockConnectionStatusListener);

    // When: call onConnectionReleased, listenerInvalidated is true by ConnectionStatusListener::onIdle.
    // Then: return true, call ConnectionStatusListener, status is Idle, remove ConnectionStatusListener.
    pConnectionInternal->onConnectionReleased();

    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
    EXPECT_TRUE(pConnectionInternal->getConnectionStatusListener() != NULL);
}

// 1. status == Inuse
// 2. setConnectionStateListener が登録されてない。
// 確認内容
// 1. true が返る。
// 2. ConnectionStatusListener::onIdle は呼び出されない。
// 3. status が Idle になる。
// 4. ConnectionPoolInternal::startKeepAliveTimer が呼び出される。。
TEST_F(ConnectionInternalUnitTest,
        onConnectionReleased_ReturnsTrueAndNotCallConnectionStatusListener_WhenStatusIsInuseAndDoesnotSetConnectionStatusListener)
{
    // Given: status is Inuse, does not set ConnectionStatusListener.
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // When: call onConnectionReleased.
    // Then: return true, status is Idle.
    EXPECT_TRUE(pConnectionInternal->onConnectionReleased());
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
}

// 1. status == Idle
// 2. setConnectionStateListener が登録されてる
// 確認内容
// 1. false が返る。
// 2. ConnectionPoolInternal::startKeepAliveTimer が呼び出されない。
TEST_F(ConnectionInternalUnitTest,
        onConnectionReleased_ReturnsFalseAndNotCallStartKeepAliveTimer_WhenStatusIsIdleAndSetConnectionStatusListener)
{
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Given: status is Idle, set ConnectionStatusListener.
    ASSERT_TRUE(pConnectionInternal->onConnectionReleased());
    ASSERT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());

    MockConnectionStatusListener mockConnectionStatusListener;
    EXPECT_CALL(mockConnectionStatusListener, onIdle(pConnectionInternal.get(), testing::_))
            .WillOnce(testing::SetArgReferee<1>(true));
    pConnectionInternal->setConnectionStatusListener(&mockConnectionStatusListener);

    // When: call onConnectionReleased.
    // Then: return false, status is Idle.
    EXPECT_FALSE(pConnectionInternal->onConnectionReleased());
    EXPECT_EQ(ConnectionInternal::Idle, pConnectionInternal->getStatus());
}

// 既にcancel が呼び出されている場合の cancel
// true が返る。
TEST_F(ConnectionInternalUnitTest, cancel_ReturnsTrue_WhenAfterCallCancel)
{
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Given: 一度 cancel を呼び出す。
    // まだ通信を開始していないので、このcancel は、失敗する。
    ASSERT_FALSE(pConnectionInternal->cancel());

    // When: call cancel
    // Then: return true
    EXPECT_TRUE(pConnectionInternal->cancel());
}

// cancel で Poco::Exception が発生
// false が返る。
TEST_F(ConnectionInternalUnitTest, cancel_ReturnsFalse_WhenOccurredPocoException)
{
    PocoHttpClientSessionPtr pPocoHttpClientSession = new Poco::Net::HTTPClientSession();
    std::string url = TestDefaultUrl;
    Proxy::Ptr pProxy = new Proxy(ProxyName, ProxyPort);
    EasyHttpContext::Ptr pEasyHttpContext = new EasyHttpContext();
    ConnectionInternal::Ptr pConnectionInternal = new ConnectionInternal(pPocoHttpClientSession, url, pEasyHttpContext);

    // Given: HTTPClientSession の sendRequest を呼び出さない。
    // When: cancel を呼び出す。socket::shutdown で Poco::Exception が発生する。
    // Then: return false
    EXPECT_FALSE(pConnectionInternal->cancel());
}

} /* namespace test */
} /* namespace easyhttpcpp */
