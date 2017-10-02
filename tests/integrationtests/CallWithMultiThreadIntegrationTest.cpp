/*
 * Copyright 2017 Sony Corporation
 */

#include <string>

#include "gtest/gtest.h"

#include "Poco/Event.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/HashMap.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Path.h"
#include "Poco/RefCountedObject.h"
#include "Poco/String.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/EasyHttp.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Interceptor.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"
#include "HttpTestServer.h"

#include "HttpCacheDatabase.h"
#include "HttpIntegrationTestCase.h"
#include "HttpTestBaseRequestHandler.h"
#include "HttpTestConstants.h"
#include "HttpTestUtil.h"
#include "HttpUtil.h"

using easyhttpcpp::common::FileUtil;
using easyhttpcpp::common::StringUtil;
using easyhttpcpp::testutil::HttpTestServer;
using easyhttpcpp::testutil::HttpTestRequestHandlerFactory;

namespace easyhttpcpp {
namespace test {

static const std::string Tag = "CallWithMultiThreadIntegrationTest";
static const char* const Execute1Path = "/path1";
static const char* const Execute2Path = "/path2";
static const size_t ResponseBufferBytes = 8192;
static const int TestFailureTimeout = 10 * 1000; // milliseconds
static const int MultiThreadCount = 10;

namespace {

class SyncRequestHandler : public Poco::Net::HTTPRequestHandler, public Poco::RefCountedObject {
public:

    SyncRequestHandler(long eventTimeout, int number) : m_number(number), m_eventTimeout(eventTimeout),
            m_handlerExecuted(false)
    {
        m_handlerId = Poco::NumberFormatter::format(m_number);
        m_responseBody = m_handlerId + ":" + HttpTestConstants::DefaultResponseBody;
    }

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
        m_handlerExecuted = true;
        setToStartHandler();
        waitToExecuteHandler();

        response.setContentType(HttpTestConstants::DefaultResponseContentType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentLength(getResponseBodySize());

        std::ostream& ostr = response.send();
        ostr << m_responseBody;
    }

    bool waitToStartHandler()
    {
        return m_startEvent.tryWait(m_eventTimeout);
    }

    void setToExecuteHandler()
    {
        m_executeEvent.set();
    }

    const std::string& getResponseBody() const
    {
        return m_responseBody;
    }

    size_t getResponseBodySize() const
    {
        return m_responseBody.length();
    }

    int getNumber() const
    {
        return m_number;
    }

    std::string getHandlerId()
    {
        return m_handlerId;
    }

    bool isHandlerExecuted() const
    {
        return m_handlerExecuted;
    }

    void setPath(const std::string path)
    {
        m_path = path;
    }

    const std::string& getPath() const
    {
        return m_path;
    }

protected:
    void setToStartHandler()
    {
        m_startEvent.set();
    }

    bool waitToExecuteHandler()
    {
        return m_executeEvent.tryWait(m_eventTimeout);
    }

private:
    int m_number;
    long m_eventTimeout;
    std::string m_responseBody;
    Poco::Event m_startEvent;
    Poco::Event m_executeEvent;
    bool m_handlerExecuted;
    std::string m_handlerId;
    std::string m_path;
};

class HttpExecutionRunner : public Poco::Runnable, public Poco::RefCountedObject {
public:

    HttpExecutionRunner(const std::string& path, SyncRequestHandler& handler, HttpCache::Ptr pCache) :
            m_path(path), m_handler(handler), m_pCache(pCache), m_isSuccess(false)
    {
    }

    virtual void run()
    {
        m_isSuccess = execute();
        setToFinish();
    }

    virtual bool execute()
    {
        bool ret = true;

        // create EasyHttp
        EasyHttp::Builder httpClientBuilder;
        if (!m_pCache.isNull()) {
            httpClientBuilder.setCache(m_pCache);
        }
        EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
        Request::Builder requestBuilder;
        m_url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, m_path, HttpTestConstants::DefaultQuery);
        Request::Ptr pRequest = requestBuilder.setUrl(m_url).
                setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, m_handler.getHandlerId()).build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        // execute GET method.
        if (!waitToExecute()) {
            ret = false;
        }
        Response::Ptr pResponse = pCall->execute();

        // check response parameter
        if (pResponse->getCode() != 200) {
            setErrorMessage(StringUtil::format("status code is wrong.[%d] no=%d", pResponse->getCode(),
                    m_handler.getNumber()));
            ret = false;
        }

        // read response body
        ResponseBody::Ptr pResponseBody = pResponse->getBody();
        ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
        Poco::Buffer<char> buffer(ResponseBufferBytes);
        size_t retBytes =  HttpTestUtil::readAllData(pResponseBodyStream, buffer);
        if (retBytes != m_handler.getResponseBodySize()) {
            setErrorMessage(StringUtil::format("response body size is wrong.[%zu] no=%d", retBytes,
                    m_handler.getNumber()));
            ret = false;
        }
        std::string resBody(buffer.begin(), m_handler.getResponseBodySize());
        if (resBody != m_handler.getResponseBody()) {
            setErrorMessage(StringUtil::format("response body is wrong.[%s] no=%d",
                    resBody.c_str(), m_handler.getNumber()));
            ret = false;
        }

        if (!waitToClose()) {
            ret = false;
        }
        pResponseBodyStream->close();

        return ret;
    }

    bool waitToFinish()
    {
        return m_finishEvent.tryWait(TestFailureTimeout);
    }

    bool isSuccess()
    {
        return m_isSuccess;
    }

    // test の verify (EXPECT_XXX) で表示するメッセージを設定します。
    void setErrorMessage(const std::string& message)
    {
        EASYHTTPCPP_LOG_E(Tag, "%s", message.c_str());
        if (m_message.empty()) {
            m_message = message;
        }
    }

    std::string getErrorMessage()
    {
        return m_message;
    }

    const std::string& getUrl()
    {
        return m_url;
    }

protected:
    virtual bool waitToExecute()
    {
        return true;
    }

    virtual bool waitToClose()
    {
        return true;
    }

protected:
    std::string m_path;
    SyncRequestHandler& m_handler;
    HttpCache::Ptr m_pCache;
    std::string m_url;
    bool m_isSuccess;
    bool m_syncExecute;

private:
    void setToFinish()
    {
        m_finishEvent.set();
    }

    Poco::Event m_finishEvent;
    std::string m_message;
};

class HttpSyncExecutionRunner : public HttpExecutionRunner {
public:

    HttpSyncExecutionRunner(const std::string& path, SyncRequestHandler& handler, HttpCache::Ptr pCache) :
            HttpExecutionRunner(path, handler, pCache)
    {
    }

    bool waitBeforeExecute()
    {
        return m_beforeExecuteEvent.tryWait(TestFailureTimeout);
    }

    void setStartToExecute()
    {
        m_startToExecuteEvent.set();
    }

protected:
    virtual bool waitToExecute()
    {
        m_beforeExecuteEvent.set();
        if (!m_startToExecuteEvent.tryWait(TestFailureTimeout)) {
                setErrorMessage(StringUtil::format("start to execute event is time out. no=%d",
                        m_handler.getNumber()));
            return false;
        }
        return true;
    }

protected:
    Poco::Event m_beforeExecuteEvent;
    Poco::Event m_startToExecuteEvent;
};

class HttpExecutionSyncCloseResponseBodyRunner : public HttpExecutionRunner {
public:

    HttpExecutionSyncCloseResponseBodyRunner(const std::string& path, SyncRequestHandler& handler,
            HttpCache::Ptr pCache) : HttpExecutionRunner(path, handler, pCache)
    {
    }

    void setSyncResponseBodyClose()
    {
        m_syncResponseBodyCloseEvent.set();
    }

    bool waitBeforeSyncResponseBodyClose()
    {
        return  m_beforeSyncResponseBodyCloseEvent.tryWait(TestFailureTimeout);
    }

protected:
    virtual bool waitToClose()
    {
        m_beforeSyncResponseBodyCloseEvent.set();
        if (!m_syncResponseBodyCloseEvent.tryWait(TestFailureTimeout)) {
            setErrorMessage(StringUtil::format("response body close event is time out. no=%d",
                    m_handler.getNumber()));
            return false;
        }
        return true;
    }

protected:
    Poco::Event m_beforeSyncResponseBodyCloseEvent;
    Poco::Event m_syncResponseBodyCloseEvent;
};

class HttpSyncExecuteForceCacheRunner : public HttpSyncExecutionRunner {
public:

    HttpSyncExecuteForceCacheRunner(const std::string& path, SyncRequestHandler& handler, HttpCache::Ptr pCache) :
            HttpSyncExecutionRunner(path, handler, pCache)
    {
    }

    virtual bool execute()
    {
        bool ret = true;

        // create EasyHttp
        EasyHttp::Builder httpClientBuilder;
        if (!m_pCache.isNull()) {
            httpClientBuilder.setCache(m_pCache);
        }
        EasyHttp::Ptr pHttpClient = httpClientBuilder.build();
        Request::Builder requestBuilder;
        m_url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, m_path, HttpTestConstants::DefaultQuery);
        CacheControl::Ptr requestCacheControl = CacheControl::createForceCache();
        Request::Ptr pRequest = requestBuilder.setUrl(m_url).
                setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, m_handler.getHandlerId()).
                setCacheControl(requestCacheControl).build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        // execute GET method.
        m_beforeExecuteEvent.set();
        if (!m_startToExecuteEvent.tryWait(TestFailureTimeout)) {
                setErrorMessage(StringUtil::format("start to execute event is time out. no=%d",
                        m_handler.getNumber()));
            ret = false;
        }
        Response::Ptr pResponse = pCall->execute();

        // check response parameter
        if (pResponse->getCode() != 200) {
            setErrorMessage(StringUtil::format("status code is wrong.[%d] no=%d", pResponse->getCode(),
                    m_handler.getNumber()));
            ret = false;
        }

        // check request parameter
        if (m_handler.isHandlerExecuted()) {
            setErrorMessage(StringUtil::format("network access no=%d", m_handler.getNumber()));
            ret = false;
        }

        // read response body and close
        pResponse->getBody()->toString();

        return ret;
    }
};

} /* namespace */

class CallWithMultiThreadIntegrationTest : public HttpIntegrationTestCase {
protected:

    void SetUp()
    {
        Poco::Path path(HttpTestUtil::getDefaultCachePath());
        FileUtil::removeDirsIfPresent(path);
    }

    Poco::AutoPtr<SyncRequestHandler> m_pHandlers[MultiThreadCount];
    Poco::AutoPtr<HttpExecutionRunner> m_pExecutes[MultiThreadCount];
};

// HttpCache なしで、multi Thread からの同時execute 要求
TEST_F(CallWithMultiThreadIntegrationTest, execute_ReturnsResponse_WhenNotUseCacheAndExecuteAtTheSameTimeOnMultiThread)
{
    // Given: execute1 wait start of handleRequest.

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    Poco::ThreadPool threadPool;
    SyncRequestHandler handler1(TestFailureTimeout, 0);
    testServer.getTestRequestHandlerFactory().addHandler(Execute1Path, &handler1,
            Poco::NumberFormatter::format(handler1.getNumber()));
    HttpExecutionRunner execute1(Execute1Path, handler1, NULL);
    SyncRequestHandler handler2(TestFailureTimeout, 1);
    testServer.getTestRequestHandlerFactory().addHandler(Execute2Path, &handler2,
            Poco::NumberFormatter::format(handler2.getNumber()));
    HttpExecutionRunner execute2(Execute2Path, handler2, NULL);
    threadPool.start(execute1);
    handler1.waitToStartHandler(); // wait until execute1 handler started.
                                   // execute1 wait until setExecuteHandler.

    // When: execute2 call execute. start execute2. wait until execute2 finish.
    threadPool.start(execute2);
    handler2.setToExecuteHandler(); // execute2 do not wait.
    execute2.waitToFinish();        // wait until execute2 finished.
    handler1.setToExecuteHandler(); // execute1 execute handler.

    threadPool.joinAll();

    // Then: execute succeeded.
    EXPECT_TRUE(execute1.isSuccess()) << execute1.getErrorMessage();
    EXPECT_TRUE(execute2.isSuccess()) << execute2.getErrorMessage();
}

// 同じurlで multi threadから同時request の場合、後で受信したresponse body がCache に格納される
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_StoresToCacheLatestResponse_WhenUseCacheAndExecuteAtTheSameTimeOnMultiThread)
{
    // Given: use cache. wait start of handleRequest of execute1.

    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    Poco::ThreadPool threadPool;
    SyncRequestHandler handler1(TestFailureTimeout, 0);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler1,
            handler1.getHandlerId());
    HttpExecutionRunner execute1(HttpTestConstants::DefaultPath, handler1, pCache);
    SyncRequestHandler handler2(TestFailureTimeout, 1);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler2,
            handler2.getHandlerId());
    HttpExecutionRunner execute2(HttpTestConstants::DefaultPath, handler2, pCache);
    threadPool.start(execute1);
    handler1.waitToStartHandler(); // wait until execute1 handler started.
    // execute1 wait until setExecuteHandler.

    // When: execute2 call execute. wait until execute1 finish. start execute2.
    threadPool.start(execute2);
    handler2.waitToStartHandler(); // wait until execute2 handler started.
    handler1.setToExecuteHandler(); // execute1 do not wait.
    execute1.waitToFinish(); // wait until execute2 finished.
    handler2.setToExecuteHandler(); // execute2 execute handler.

    threadPool.joinAll();

    // Then: response of execute2 store to cache.
    EXPECT_TRUE(execute1.isSuccess()) << execute1.getErrorMessage();
    EXPECT_TRUE(execute2.isSuccess()) << execute2.getErrorMessage();

    const std::string& url = execute1.getUrl();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // latest response body (handler2, execute2) is cached.
    size_t expectResponseBodySize = handler2.getResponseBodySize();
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    ASSERT_EQ(expectResponseBodySize, responseBodyFile.getSize());
    Poco::FileInputStream responseBodyStream(responseBodyFile.path(), std::ios::in | std::ios::binary);
    Poco::Buffer<char> inBuffer(expectResponseBodySize);
    responseBodyStream.read(inBuffer.begin(), expectResponseBodySize);
    ASSERT_EQ(expectResponseBodySize, responseBodyStream.gcount());
    ASSERT_EQ(0, memcmp(inBuffer.begin(), handler2.getResponseBody().c_str(), expectResponseBodySize));
}

// 同じ url で、receive response が multi thread でじっこうされても、通信は成功する
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_ReturnsResponse_WhenUseCacheAndReceivedResponseOnMultiThread)
{
    // Given: use cache. each thread call execute and read response body and wait before close.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, m_pHandlers[i],
                m_pHandlers[i]->getHandlerId());
        m_pExecutes[i] = new HttpExecutionSyncCloseResponseBodyRunner(HttpTestConstants::DefaultPath, *m_pHandlers[i],
                pCache);
    }

    // execute until before close response body.
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i]->setToExecuteHandler();
        threadPool.start(*m_pExecutes[i]);
    }

    // When: each thread execute close of response body.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(static_cast<HttpExecutionSyncCloseResponseBodyRunner*>(m_pExecutes[i].get())
                ->waitBeforeSyncResponseBodyClose());
        static_cast<HttpExecutionSyncCloseResponseBodyRunner*>(m_pExecutes[i].get())->setSyncResponseBodyClose();
    }

    threadPool.joinAll();

    // Then: request succeeded on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << m_pExecutes[i]->getErrorMessage();
    }

    // latest response is cached.
    const std::string& url = m_pExecutes[0]->getUrl();
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // latest response body is cached.
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_TRUE(responseBodyFile.exists());
}
// Cache があり、同じurlで multi threadから同時に ForceCached のrequest がじっこうされても、通信は成功する。
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_ReturnsResponse_WhenUseCacheAndExistCachedResponseAndForceCachedOnMultiThread)
{
    // Given: use cache. create 1 url in cache.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    // make cache response
    SyncRequestHandler handler(TestFailureTimeout, 100);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler,
            handler.getHandlerId());

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).
            setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handler.getHandlerId()).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    handler.setToExecuteHandler();
    Response::Ptr pResponse = pCall->execute();
    ASSERT_TRUE(pResponse->isSuccessful());

    // read response body and close
    std::string responseBody = pResponse->getBody()->toString();

    // check database
    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // http request by thread
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, m_pHandlers[i],
                m_pHandlers[i]->getHandlerId());
        m_pExecutes[i] = new HttpSyncExecuteForceCacheRunner(HttpTestConstants::DefaultPath, *m_pHandlers[i], pCache);
    }

    // When: execute (forceCache)
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i]->setToExecuteHandler(); // もし、network access がはっせいした時のために、handler は、wait しない。
        threadPool.start(*m_pExecutes[i]);
        EXPECT_TRUE(static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->waitBeforeExecute())
                << StringUtil::format("no=%d", i);
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->setStartToExecute();
    }

    threadPool.joinAll();

    // Then: request succeeded on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << m_pExecutes[i]->getErrorMessage();
    }

    HttpCacheDatabase::HttpCacheMetadataAll metadata2;
    EXPECT_TRUE(db.getMetadataAll(key, metadata2));

    // latest response body is cached.
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_TRUE(responseBodyFile.exists());

    // cached response body not changed from original
    size_t responseBodySize = handler.getResponseBodySize();
    ASSERT_EQ(responseBodySize, responseBodyFile.getSize());
    Poco::FileInputStream responseBodyStream(responseBodyFile.path(), std::ios::in | std::ios::binary);
    Poco::Buffer<char> inBuffer(responseBodySize);
    responseBodyStream.read(inBuffer.begin(), responseBodySize);
    ASSERT_EQ(responseBodySize, responseBodyStream.gcount());
    ASSERT_EQ(0, memcmp(inBuffer.begin(), handler.getResponseBody().c_str(), responseBodySize));
}

// Cache があり、同じurlで multi threadから同時にrequest がじっこうされても、通信は成功する。(Network アクセスあり)
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_ReturnsResponse_WhenUseCacheAndExistCachedResponseAndExecuteNetworkAccessOnMultiThread)
{
    // Given: use cache. create 1 url in cache.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    SyncRequestHandler handler(TestFailureTimeout, 100);
    testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, &handler,
            handler.getHandlerId());

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();
    Request::Builder requestBuilder;
    std::string url = HttpTestConstants::DefaultTestUrlWithQuery;
    Request::Ptr pRequest = requestBuilder.setUrl(url).
            setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, handler.getHandlerId()).build();
    Call::Ptr pCall = pHttpClient->newCall(pRequest);

    // execute GET method.
    handler.setToExecuteHandler();
    Response::Ptr pResponse = pCall->execute();
    ASSERT_TRUE(pResponse->isSuccessful());

    // read response body and close. create cache.
    std::string responseBody = pResponse->getBody()->toString();
    
    // http request by thread
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        testServer.getTestRequestHandlerFactory().addHandler(HttpTestConstants::DefaultPath, m_pHandlers[i],
                m_pHandlers[i]->getHandlerId());
        m_pExecutes[i] = new HttpSyncExecutionRunner(HttpTestConstants::DefaultPath, *m_pHandlers[i], pCache);
    }

    // When: execute on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i]->setToExecuteHandler();
        threadPool.start(*m_pExecutes[i]);
        EXPECT_TRUE(static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->waitBeforeExecute())
                << StringUtil::format("no=%d", i);
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->setStartToExecute();
    }

    threadPool.joinAll();

    // Then: request succeeded on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << m_pExecutes[i]->getErrorMessage();
    }

    HttpCacheDatabase db(HttpTestUtil::createDatabasePath(cachePath));
    HttpCacheDatabase::HttpCacheMetadataAll metadata1;
    std::string key = HttpUtil::makeCacheKey(Request::HttpMethodGet, url);
    EXPECT_TRUE(db.getMetadataAll(key, metadata1));

    // latest response body is cached.
    Poco::File responseBodyFile(HttpTestUtil::createCachedResponsedBodyFilePath(cachePath,
            Request::HttpMethodGet, url));
    EXPECT_TRUE(responseBodyFile.exists());
}

// それぞれ Cache があるじょうたいで、べつべつの url で multi threadから同時に ForceCached のrequest がじっこうされても、
// 通信は成功する。
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_ReturnsResponse_WhenUseCacheAndExistCachedResponseAndForceCachedOnMultiThreadWithEachUrl)
{
    // Given: use cache. create each url in cache.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        m_pHandlers[i]->setPath(StringUtil::format("%s%s", HttpTestConstants::DefaultPath,
                m_pHandlers[i]->getHandlerId().c_str()));
        testServer.getTestRequestHandlerFactory().addHandler(m_pHandlers[i]->getPath(),
                m_pHandlers[i], m_pHandlers[i]->getHandlerId());

        Request::Builder requestBuilder;
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, m_pHandlers[i]->getPath(), HttpTestConstants::DefaultQuery);
        Request::Ptr pRequest = requestBuilder.setUrl(url).
                setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, m_pHandlers[i]->getHandlerId()).build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        // execute GET method.
        m_pHandlers[i]->setToExecuteHandler();
        Response::Ptr pResponse = pCall->execute();
        ASSERT_TRUE(pResponse->isSuccessful());

        // read response body and close
        std::string responseBody = pResponse->getBody()->toString();
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = NULL;
    }

    // ForceCache request by each url
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        m_pHandlers[i]->setPath(StringUtil::format("%s%s", HttpTestConstants::DefaultPath,
                m_pHandlers[i]->getHandlerId().c_str()));
        m_pExecutes[i] = new HttpSyncExecuteForceCacheRunner(m_pHandlers[i]->getPath(), *m_pHandlers[i], pCache);
    }

    // When: execute each thread
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i]->setToExecuteHandler(); // もし、network access がfはっせいした時のために、handler は、wait しない。
        threadPool.start(*m_pExecutes[i]);
        EXPECT_TRUE(static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->waitBeforeExecute())
                << StringUtil::format("no=%d", i);
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->setStartToExecute();
    }

    threadPool.joinAll();

    // Then: request succeeded on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << m_pExecutes[i]->getErrorMessage();
    }
}

// それぞれ Cache があるじょうたいで、べつべつの url で multi threadから同時に request がじっこうされても、
// 通信は成功する。(Network アクセスあり)
TEST_F(CallWithMultiThreadIntegrationTest,
        execute_ReturnsResponse_WhenUseCacheAndExistCachedResponseAndExecuteNetworkAccessOnMultiThreadWithEachUrl)
{
    // Given: use cache. create each url in cache.
    std::string cachePath = HttpTestUtil::getDefaultCachePath();
    HttpCache::Ptr pCache = HttpCache::createCache(cachePath, HttpTestConstants::DefaultCacheMaxSize);

    HttpTestServer testServer;
    testServer.start(HttpTestConstants::DefaultPort);

    EasyHttp::Builder httpClientBuilder;
    EasyHttp::Ptr pHttpClient = httpClientBuilder.setCache(pCache).build();

    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        m_pHandlers[i]->setPath(StringUtil::format("%s%s", HttpTestConstants::DefaultPath,
                m_pHandlers[i]->getHandlerId().c_str()));
        testServer.getTestRequestHandlerFactory().addHandler(m_pHandlers[i]->getPath(),
                m_pHandlers[i], m_pHandlers[i]->getHandlerId());

        Request::Builder requestBuilder;
        std::string url = HttpTestUtil::makeUrl(HttpTestConstants::Http, HttpTestConstants::DefaultHost,
                HttpTestConstants::DefaultPort, m_pHandlers[i]->getPath(), HttpTestConstants::DefaultQuery);
        Request::Ptr pRequest = requestBuilder.setUrl(url).
                setHeader(HttpTestRequestHandlerFactory::HeaderHandlerId, m_pHandlers[i]->getHandlerId()).build();
        Call::Ptr pCall = pHttpClient->newCall(pRequest);

        // execute GET method.
        m_pHandlers[i]->setToExecuteHandler();
        Response::Ptr pResponse = pCall->execute();
        ASSERT_TRUE(pResponse->isSuccessful());

        // read response body and close
        std::string responseBody = pResponse->getBody()->toString();
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = NULL;
    }
    testServer.getTestRequestHandlerFactory().clearAllHandler();

    // network request by each url
    Poco::ThreadPool threadPool;
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i] = new SyncRequestHandler(TestFailureTimeout, i);
        m_pHandlers[i]->setPath(StringUtil::format("%s%s", HttpTestConstants::DefaultPath,
                m_pHandlers[i]->getHandlerId().c_str()));
        testServer.getTestRequestHandlerFactory().addHandler(m_pHandlers[i]->getPath(),
                m_pHandlers[i], m_pHandlers[i]->getHandlerId());
        m_pExecutes[i] = new HttpSyncExecutionRunner(m_pHandlers[i]->getPath(), *m_pHandlers[i], pCache);
    }

    // When: execute each thread
    for (int i = 0; i < MultiThreadCount; i++) {
        m_pHandlers[i]->setToExecuteHandler();
        threadPool.start(*m_pExecutes[i]);
        EXPECT_TRUE(static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->waitBeforeExecute())
                << StringUtil::format("no=%d", i);
    }
    for (int i = 0; i < MultiThreadCount; i++) {
        static_cast<HttpSyncExecutionRunner*>(m_pExecutes[i].get())->setStartToExecute();
    }

    threadPool.joinAll();

    // Then: request succeeded on each thread.
    for (int i = 0; i < MultiThreadCount; i++) {
        EXPECT_TRUE(m_pExecutes[i]->isSuccess()) << m_pExecutes[i]->getErrorMessage();
    }
}

} /* namespace test */
} /* namespace easyhttpcpp */
