/*
 * Copyright 2017 Sony Corporation
 */

#include <istream>
#include <ostream>

#include "Poco/Buffer.h"
#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"
#include "Poco/SharedPtr.h"
#include "Poco/String.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/SSLException.h"

#include "easyhttpcpp/common/CacheManager.h"
#include "easyhttpcpp/common/CacheMetadata.h"
#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"

#include "HttpCacheInternal.h"
#include "HttpCacheMetadata.h"
#include "HttpEngine.h"
#include "HttpUtil.h"
#include "NetworkInterceptorChain.h"
#include "ResponseBodyStreamFromCache.h"
#include "ResponseBodyStreamWithoutCaching.h"
#include "ResponseBodyStreamWithCaching.h"

using easyhttpcpp::common::CacheManager;
using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

#define DEFAULT_CONTENT_TYPE HttpConstants::HeaderValues::ApplicationOctetStream    // RFC2616 7.2.1

static const std::string Tag = "HttpEngine";
static const bool EnableRedirect = true;
static const bool EnableSchemeChangedRedirect = false;
static const size_t TempResponseBufferBytes = 1024;

HttpEngine::HttpEngine(EasyHttpContext::Ptr pContext, Request::Ptr pRequest, Response::Ptr pPriorResponse) :
        m_pContext(pContext), m_pUserRequest(pRequest), m_pPriorResponse(pPriorResponse), m_cancelled(false),
        m_connectionRetried(false)
{
}

HttpEngine::~HttpEngine()
{
    if (m_pConnectionInternal) {
        m_pConnectionInternal->setConnectionStatusListener(NULL);
        m_pConnectionInternal = NULL;
    }
}

Response::Ptr HttpEngine::execute()
{
    Response::Ptr pUserResponse;
    Request::Ptr pNetworkRequest;
    // find cache candidate and check to be able to use cache.
    checkCacheBeforeSendRequest(pUserResponse, pNetworkRequest);
    if (pUserResponse) {
        EASYHTTPCPP_LOG_D(Tag, "execute: do not access network because use cache.");
        return pUserResponse;
    }

    // execute sendRequest and receiveResponse
    Response::Ptr pNetworkResponse;
    EasyHttpContext::InterceptorList& networkInterceptors = m_pContext->getNetworkInterceptors();
    EasyHttpContext::InterceptorList::iterator it = networkInterceptors.begin();
    EasyHttpContext::InterceptorList::const_iterator itEnd = networkInterceptors.end();
    if (it == itEnd) {
        pNetworkResponse = sendRequestAndReceiveResponseWithRetryByConnection(pNetworkRequest);
    } else {
        NetworkInterceptorChain::Ptr chain(new NetworkInterceptorChain(*this, pNetworkRequest, it, itEnd));
        pNetworkResponse = (*it)->intercept(*chain);
    }

    // if not exist cache, create user response from network response.
    if (!m_pContext->getCache()) {
        EASYHTTPCPP_LOG_D(Tag, "return from network response.(no http cache)");
        return createUserResponseFromNetworkResponse(pNetworkResponse);
    }

    // determine cache response can be used.
    if (m_pCacheResponse) {
        if (HttpCacheStrategy::isValidCacheResponse(m_pCacheResponse, pNetworkResponse)) {
            EASYHTTPCPP_LOG_D(Tag, "execute: return from cache.(Not Modified or cache is fresh than network response.)");
            // create user response from cache response.
            pUserResponse = createUserResponseFromCacheResponse(m_pCacheResponse, pNetworkResponse);
            return pUserResponse;
        }
    }

    // check cache condition
    if (HttpCacheStrategy::isCacheable(pNetworkResponse)) {
        // create user response from network response with caching.
        EASYHTTPCPP_LOG_D(Tag, "execute: return from cache response.");
        return createUserResponseWithCaching(pNetworkResponse);
    } else {
        if (HttpCacheStrategy::isInvalidCacheMethod(pNetworkResponse)) {
            // remove from cache (POST etc. method)
            EASYHTTPCPP_LOG_D(Tag, "execute: remove cache");
            removeFromCache(pNetworkResponse);
        }
        // create user response from network response without caching.
        EASYHTTPCPP_LOG_D(Tag, "execute: return from network response.(not cacheable)");
        return createUserResponseFromNetworkResponse(pNetworkResponse);
    }
}

Response::Ptr HttpEngine::stripBody(Response::Ptr pResponse)
{
    if (!pResponse) {
        return NULL;
    }
    ResponseBody::Ptr pBody = pResponse->getBody();
    if (!pBody) {
        return pResponse;
    }
    Response::Builder builder(pResponse);
    return builder.setBody(NULL).build();
}

Request::Ptr HttpEngine::makeRetryRequest(Response::Ptr pResponse)
{
    // check Location
    if (!pResponse->hasHeader(HttpConstants::HeaderNames::Location)) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: No Location header.");
        return NULL;
    }
    std::string redirectUrl = pResponse->getHeaderValue(HttpConstants::HeaderNames::Location, "");
    if (redirectUrl.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: Location value is empty.");
        return NULL;
    }
    std::string resolvedUrl;
    try {
        Poco::URI::decode(redirectUrl, resolvedUrl);
    } catch (const Poco::Exception&) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: redirect url can not decode [%s]", redirectUrl.c_str());
        return NULL;
    }
    if (resolvedUrl.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: resolved url is empty. redirectUrl=[%s]", redirectUrl.c_str());
        return NULL;
    }

    // check scheme
    Request::Ptr pRequest = pResponse->getRequest();
    Poco::URI beforeUri(pRequest->getUrl());
    Poco::URI resolvedUri(resolvedUrl);
    if (beforeUri.getScheme() != resolvedUri.getScheme() && !EnableSchemeChangedRedirect) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: redirect scheme is changed. (not allowed to change scheme)");
        return NULL;
    }

    // remove Authorization TODO: confirm RFC
    Headers::Ptr pRequestHeaders = pRequest->getHeaders();
    Headers::Ptr pRetryHeaders = new Headers();
    for (Headers::HeaderMap::ConstIterator it = pRequestHeaders->begin(); it != pRequestHeaders->end(); it++) {
        if (Poco::icompare(it->first, HttpConstants::HeaderNames::Authorization) == 0) {
            if (beforeUri.getScheme() != resolvedUri.getScheme()) {
                EASYHTTPCPP_LOG_D(Tag, "makeRetryRequest: remove Authorization Header because change scheme.[%s] -> [%s]",
                        beforeUri.getScheme().c_str(), resolvedUri.getScheme().c_str());
                continue;
            }
            if (beforeUri.getHost() != resolvedUri.getHost()) {
                EASYHTTPCPP_LOG_D(Tag, "makeRetryRequest: remove Authorization Header because change host.[%s] -> [%s]",
                        beforeUri.getHost().c_str(), resolvedUri.getHost().c_str());
                continue;
            }
            if (beforeUri.getPort() != resolvedUri.getPort()) {
                EASYHTTPCPP_LOG_D(Tag, "makeRetryRequest: remove Authorization Header because change port.[%u] -> [%u]",
                        beforeUri.getPort(), resolvedUri.getPort());
                continue;
            }
        }
        pRetryHeaders->add(it->first, it->second);
    }
    Request::Builder retryRequestBuilder(pRequest);
    return retryRequestBuilder.setUrl(resolvedUrl).setHeaders(pRetryHeaders).build();
}

void HttpEngine::checkCacheBeforeSendRequest(Response::Ptr& pUserResponse, Request::Ptr& pNetworkRequest)
{
    pUserResponse = NULL;
    pNetworkRequest = NULL;

    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pContext->getCache().get());
    if (pCacheInternal == NULL) {
        EASYHTTPCPP_LOG_D(Tag, "HttpCache is not used.");
        pNetworkRequest = m_pUserRequest;
        return;
    }

    if (!HttpCacheStrategy::isAvailableToCache(m_pUserRequest)) {
        EASYHTTPCPP_LOG_D(Tag, "request is not available to cache.");
        pNetworkRequest = m_pUserRequest;
        return;
    }

    EASYHTTPCPP_LOG_D(Tag, "request is available to cache.");
    HttpCacheStrategy::Ptr pCacheStrategy = pCacheInternal->createCacheStrategy(m_pUserRequest);
    pNetworkRequest = pCacheStrategy->getNetworkRequest();
    m_pCacheResponse = pCacheStrategy->getCachedResponse();

    // when no networkRequest and no cacheResponse, return 504.
    if (!pNetworkRequest && !m_pCacheResponse) {
        EASYHTTPCPP_LOG_D(Tag, "return 504 because network request and cache response not found.");
        Poco::Timestamp now;
        Response::Builder builder;
        pUserResponse = builder.setRequest(m_pUserRequest)
                .setPriorResponse(stripBody(m_pPriorResponse))
                .setCode(Poco::Net::HTTPResponse::HTTP_GATEWAY_TIMEOUT)
                .setMessage(Poco::Net::HTTPResponse::HTTP_REASON_GATEWAY_TIMEOUT)
                .setSentRequestSec(now.epochTime()).setReceivedResponseSec(now.epochTime()).build();
        return;
    }

    // use cacheResponse
    if (!pNetworkRequest) {
        EASYHTTPCPP_LOG_D(Tag, "user response is cache response.");
        pUserResponse = createUserResponseFromCacheResponse(m_pCacheResponse, NULL);
    }
}

Response::Ptr HttpEngine::sendRequestAndReceiveResponseWithRetryByConnection(Request::Ptr pNetworkRequest)
{
    bool forceToCreateConnection = false;
    for (;;) {
        bool connectionReused = false;
        try {
            return sendRequestAndReceiveResponse(pNetworkRequest, forceToCreateConnection, connectionReused);
        } catch (const HttpExecutionException& e) {
            {
                // do not call ConnectionInternal::setConnectionStatusListener with m_connectionMutex locked.
                ConnectionInternal::Ptr pConnectionInternal;
                {
                    Poco::FastMutex::ScopedLock lock(m_connectionMutex);
                    pConnectionInternal = m_pConnectionInternal;
                    m_pConnectionInternal = NULL;
                }

                if (!pConnectionInternal) {
                    EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponseWithRetryByConnection: no connection. [%s]",
                            e.getMessage().c_str());
                    throw;
                }

                m_pConnectionPoolInternal->removeConnection(pConnectionInternal);

                if (m_cancelled) {
                    EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponseWithRetryByConnection: cancelled. [%s]",
                            e.getMessage().c_str());
                    throw;
                }

                // retry when reusing Connection.
                if (!connectionReused) {
                    EASYHTTPCPP_LOG_D(Tag,
                            "sendRequestAndReceiveResponseWithRetryByConnection: new connection do not retry. [%s]",
                            e.getMessage().c_str());
                    throw;
                } else {
                    RequestBody::Ptr pRequestBody = pNetworkRequest->getBody();
                    if (pRequestBody && !pRequestBody->reset()) {
                        EASYHTTPCPP_LOG_D(Tag,
                                "sendRequestAndReceiveResponseWithRetryByConnection: can not reset request body. [%s]",
                                e.getMessage().c_str());
                        throw HttpConnectionRetryException(
                                "Request body does not support retry. please rebuild request body.", e);
                    }
                    pConnectionInternal->setConnectionStatusListener(NULL);
                    forceToCreateConnection = true;
                    m_connectionRetried = true;
                    EASYHTTPCPP_LOG_D(Tag,
                            "sendRequestAndReceiveResponseWithRetryByConnection: retry connection. [%s]",
                            e.getMessage().c_str());
                }
            }
        } catch (const HttpException& e) {
            EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponseWithRetryByConnection: other HttpException. [%s]",
                    e.getMessage().c_str());
            // do not call ConnectionInternal::setConnectionStatusListener with m_connectionMutex locked.
            ConnectionInternal::Ptr pConnectionInternal;
            {
                Poco::FastMutex::ScopedLock lock(m_connectionMutex);
                pConnectionInternal = m_pConnectionInternal;
                m_pConnectionInternal = NULL;
            }
            if (pConnectionInternal) {
                m_pConnectionPoolInternal->removeConnection(pConnectionInternal);
            }
            throw;
        }
    }
}

Response::Ptr HttpEngine::sendRequestAndReceiveResponse(Request::Ptr pNetworkRequest, bool forceToCreateConnection,
        bool& connectionReused)
{
    connectionReused = false;

    {
        Poco::FastMutex::ScopedLock lock(m_connectionMutex);
        if (m_cancelled) {
            EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponse: request is cancelled before create Connection.");
            throw HttpExecutionException("Http request is cancelled.");
        }
    }

    ConnectionPool::Ptr pConnectionPool = m_pContext->getConnectionPool();
    // if ConnectionPool is not specified, create temporary ConnectionPool.
    if (!pConnectionPool) {
        pConnectionPool = ConnectionPool::createConnectionPool();
    }
    m_pConnectionPoolInternal = pConnectionPool.unsafeCast<ConnectionPoolInternal>();

    Poco::URI uri;
    try {
        uri = pNetworkRequest->getUrl();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponse: url is not valid. [%s] Poco::Exception=[%s]",
                pNetworkRequest->getUrl().c_str(), e.message().c_str());
        throw HttpExecutionException(StringUtil::format(
                "Url is not valid. [%s] message=[%s]", pNetworkRequest->getUrl().c_str(), e.message().c_str()), e);
    }

    // get connection.
    ConnectionInternal::Ptr pConnectionInternal;
    if (forceToCreateConnection) {
        pConnectionInternal = m_pConnectionPoolInternal->createConnection(pNetworkRequest, m_pContext);
    } else {
        pConnectionInternal = m_pConnectionPoolInternal->getConnection(pNetworkRequest, m_pContext, connectionReused);
    }

    // do not call ConnectionInternal::setConnectionStatusListener with m_connectionMutex locked.
    pConnectionInternal->setConnectionStatusListener(this);

    {
        Poco::FastMutex::ScopedLock lock(m_connectionMutex);
        m_pConnectionInternal = pConnectionInternal;
    }

    // m_pConnection variable does not become invalid during this function.
    // m_pConnection variable will become invalid by ResponseBodyStream::close().
    // because of this to lock m_connectionMutex does not need.
    // if lock m_connectionMutex, can not cancel.
    PocoHttpClientSessionPtr pPocoHttpClientSession = pConnectionInternal->getPocoHttpClientSession();

    Poco::Timestamp sentRequestTime;

    // sendRequest
    sendRequest(pPocoHttpClientSession, pNetworkRequest, uri, sentRequestTime);

    {
        Poco::FastMutex::ScopedLock lock(m_connectionMutex);
        // Cancel check after connect.
        if (m_cancelled) {
            EASYHTTPCPP_LOG_D(Tag, "sendRequestAndReceiveResponse: request is cancelled after connect.");
            throw HttpExecutionException("Http request is cancelled.");
        }
    }

    // receiveResponse
    return receiveResponse(pPocoHttpClientSession, pNetworkRequest, uri, sentRequestTime);
}

void HttpEngine::sendRequest(PocoHttpClientSessionPtr pPocoHttpClientSession, Request::Ptr pNetworkRequest,
        const Poco::URI& uri, Poco::Timestamp& sentRequestTime)
{
    // create HTTPRequest and set Content-Length, Content-Type and send request
    try {
        // create HTTPRequest
        Request::HttpMethod method = pNetworkRequest->getMethod();
        std::string path(uri.getPathAndQuery());
        if (path.empty()) {
            path = "/";
        }
        PocoHttpRequestPtr pPocoHttpRequest = new Poco::Net::HTTPRequest(HttpUtil::httpMethodToString(method),
                path, Poco::Net::HTTPMessage::HTTP_1_1);
        Headers::Ptr pHeaders = pNetworkRequest->getHeaders();
        if (pHeaders) {
            for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
                pPocoHttpRequest->add(it->first, it->second);
            }
        }

        // content-type, content-length
        RequestBody::Ptr pRequestBody = pNetworkRequest->getBody();
        if (pRequestBody) {
            MediaType::Ptr pMediaType = pRequestBody->getMediaType();
            if (!pMediaType) {
                EASYHTTPCPP_LOG_D(Tag, "sendRequest: content-type is not set. [url=%s]", pNetworkRequest->getUrl().c_str());
            } else {
                pPocoHttpRequest->set(HttpConstants::HeaderNames::ContentType, pMediaType->toString());
            }
            if (pRequestBody->hasContentLength()) {
                pPocoHttpRequest->set(HttpConstants::HeaderNames::ContentLength,
                        StringUtil::format("%zu", pRequestBody->getContentLength()));
            }
        }

        // dump request
        EASYHTTPCPP_LOG_D(Tag, "Poco HTTPRequest:");
        EASYHTTPCPP_LOG_D(Tag, "method=%s", pPocoHttpRequest->getMethod().c_str());
        for (Poco::Net::NameValueCollection::ConstIterator it = pPocoHttpRequest->begin();
                it != pPocoHttpRequest->end(); it++) {
            EASYHTTPCPP_LOG_D(Tag, "%s %s", it->first.c_str(), it->second.c_str());
        }

        // send request
        sentRequestTime.update();

        std::ostream& sendingStream = pPocoHttpClientSession->sendRequest(*pPocoHttpRequest);

        // send request body
        if (pRequestBody) {
            pRequestBody->writeTo(sendingStream);
        }

        EASYHTTPCPP_LOG_D(Tag, "send request.");

    } catch (const Poco::TimeoutException& e) {
        EASYHTTPCPP_LOG_D(Tag, "sendRequest: sendRequest has timeout [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpTimeoutException(StringUtil::format("Sending request timed out. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    } catch (const Poco::Net::SSLException& e) {
        EASYHTTPCPP_LOG_D(Tag,
                "sendRequest: SSL exception. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpSslException(
                StringUtil::format("SSL error occurred in sendRequest. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag,
                "sendRequest: Poco::Exception occurred. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpExecutionException(
                StringUtil::format("IO error occurred in sendRequest. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    }
}

Response::Ptr HttpEngine::receiveResponse(PocoHttpClientSessionPtr pPocoHttpClientSession,
        Request::Ptr pNetworkRequest, const Poco::URI& uri, Poco::Timestamp& sentRequestTime)
{
    // receive response and create network response
    try {
        // receive response
        PocoHttpResponsePtr pPocoHttpResponse = new Poco::Net::HTTPResponse();
        std::istream& receivingStream = pPocoHttpClientSession->receiveResponse(*pPocoHttpResponse);
        EASYHTTPCPP_LOG_D(Tag, "receive response.");
        Poco::Timestamp receivedResponseTime;

        // if receive Connection:Close, remove connection from ConnectionPool.
        // if response header does not contain Connection, indicate keep-alive in HTTP1.1.
        // this judgment is done by Poco.
        if (!pPocoHttpResponse->getKeepAlive()) {
            EASYHTTPCPP_LOG_D(Tag, "receiveResponse: receive Connection:Close or no Connection");
            m_pConnectionPoolInternal->removeConnection(m_pConnectionInternal);
        }

        // create networkResponse
        ResponseBodyStream::Ptr pResponseBodyStream = new ResponseBodyStreamWithoutCaching(receivingStream,
                m_pConnectionInternal, m_pConnectionPoolInternal);
        MediaType::Ptr pMediaType(new MediaType(pPocoHttpResponse->get(HttpConstants::HeaderNames::ContentType,
                DEFAULT_CONTENT_TYPE)));
        ResponseBody::Ptr pResponseBody = ResponseBody::create(pMediaType, pPocoHttpResponse->hasContentLength(),
                pPocoHttpResponse->getContentLength(), pResponseBodyStream);

        Response::Builder responseBuilder;
        responseBuilder.setRequest(pNetworkRequest).setCode(pPocoHttpResponse->getStatus())
                .setMessage(pPocoHttpResponse->getReason())
                .setHasContentLength(pPocoHttpResponse->hasContentLength())
                .setContentLength(pPocoHttpResponse->getContentLength())
                .setSentRequestSec(sentRequestTime.epochTime())
                .setReceivedResponseSec(receivedResponseTime.epochTime())
                .setBody(pResponseBody);
        Headers::Ptr pHeaders = new Headers();
        for (Poco::Net::NameValueCollection::ConstIterator it = pPocoHttpResponse->begin();
                it != pPocoHttpResponse->end(); it++) {
            pHeaders->add(it->first, it->second);
        }
        responseBuilder.setHeaders(pHeaders).setCacheControl(CacheControl::createFromHeaders(pHeaders));
        Response::Ptr pNetworkResponse = responseBuilder.build();

        // dump response header
        EASYHTTPCPP_LOG_D(Tag, "Poco HTTPResponse:");
        EASYHTTPCPP_LOG_D(Tag, "status code=%d", pPocoHttpResponse->getStatus());
        for (Poco::Net::NameValueCollection::ConstIterator it = pPocoHttpResponse->begin();
                it != pPocoHttpResponse->end(); it++) {
            EASYHTTPCPP_LOG_D(Tag, "%s %s", it->first.c_str(), it->second.c_str());
        }

        return pNetworkResponse;

    } catch (const Poco::TimeoutException& e) {
        EASYHTTPCPP_LOG_D(Tag, "receiveResponse: receiveResponse has timeout [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpTimeoutException(StringUtil::format("Receiving response timed out. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    } catch (const Poco::Net::SSLException& e) {
        EASYHTTPCPP_LOG_D(Tag,
                "receiveResponse: SSL exception. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpSslException(
                StringUtil::format("SSL error occurred in receiveResponse. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag,
                "receiveResponse: Poco::Exception occurred. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str());
        throw HttpExecutionException(
                StringUtil::format("IO error occurred in receiveResponse. [scheme=%s, host=%s] message=[%s]",
                uri.getScheme().c_str(), uri.getHost().c_str(), e.message().c_str()), e);
    }
}

bool HttpEngine::cancel()
{
    ConnectionInternal::Ptr pConnectionInternal;
    {
        Poco::FastMutex::ScopedLock lock(m_connectionMutex);
        if (m_cancelled) {
            EASYHTTPCPP_LOG_D(Tag, "cancel: already cancelled");
            return true;
        }
        m_cancelled = true;
        EASYHTTPCPP_LOG_D(Tag, "cancel: cancelled");
        pConnectionInternal = m_pConnectionInternal;
    }
    if (pConnectionInternal) {
        bool ret = pConnectionInternal->cancel();
        m_pConnectionPoolInternal->removeConnection(pConnectionInternal);
        return ret;
    }
    return m_cancelled;
}

Connection::Ptr HttpEngine::getConnection()
{
    return m_pConnectionInternal.unsafeCast<Connection>();
}

void HttpEngine::readAllResponseBodyForCache(Response::Ptr pResponse)
{
    if (!m_pContext->getCache()) {
        // when no cache, not read response body.
        return;
    }
    ResponseBody::Ptr pResponseBody = pResponse->getBody();
    if (!pResponseBody) {
        // no response body
        return;
    }
    ResponseBodyStream::Ptr pResponseBodyStream = pResponseBody->getByteStream();
    if (!pResponseBodyStream) {
        // no response body stream
        return;
    }
    // read all response body for store to cache.
    Poco::Buffer<char> responseBodyBuffer(TempResponseBufferBytes);
    while (!pResponseBodyStream->isEof()) {
        pResponseBodyStream->read(responseBodyBuffer.begin(), TempResponseBufferBytes);
    }
    pResponseBodyStream->close();

    return;
}

ConnectionPool::Ptr HttpEngine::getConnectionPool()
{
    return m_pConnectionPoolInternal.unsafeCast<ConnectionPool>();
}

bool HttpEngine::isConnectionRetried()
{
    return m_connectionRetried;
}

void HttpEngine::onIdle(ConnectionInternal* pConnectionInternal, bool& listenerInvalidated)
{
    // clear reference to Connection.
    Poco::FastMutex::ScopedLock lock(m_connectionMutex);
    if (m_pConnectionInternal && pConnectionInternal == m_pConnectionInternal.get()) {
        m_pConnectionInternal = NULL;
        listenerInvalidated = true;
    } else {
        listenerInvalidated = false;
    }
}

Request::Ptr HttpEngine::getRetryRequest(Response::Ptr pResponse)
{
    if (!pResponse) {
        EASYHTTPCPP_LOG_D(Tag, "getRetryRequest: can not check retry request because pResponse is NULL.");
        return NULL;
    }

    if (!isRetryStatusCode(pResponse)) {
        return NULL;
    }

    if (!EnableRedirect) {
        EASYHTTPCPP_LOG_D(Tag, "isRetryContition: disable redirect.");
        return NULL;
    }

    return makeRetryRequest(pResponse);
}

ResponseBody::Ptr HttpEngine::createResponseBodyFromCache(Response::Ptr pCacheResponse)
{
    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pContext->getCache().get());
    if (pCacheInternal == NULL) {
        std::string message = "Can not create response body from cache because no cache.";
        EASYHTTPCPP_LOG_D(Tag, "%s", message.c_str());
        throw HttpExecutionException(message);
    }

    std::istream* pStream = pCacheInternal->createInputStreamFromCache(pCacheResponse->getRequest());
    if (pStream == NULL) {
        EASYHTTPCPP_LOG_D(Tag, "createResponseBodyFromCache: can not create stream from cache.");
        throw HttpExecutionException("Can not create response body from cache. Maybe cache is broken.");
    }
    MediaType::Ptr pMediaType(new MediaType(pCacheResponse->getHeaderValue(
            HttpConstants::HeaderNames::ContentType, DEFAULT_CONTENT_TYPE)));
    return ResponseBody::create(pMediaType, pCacheResponse->hasContentLength(), pCacheResponse->getContentLength(),
            new ResponseBodyStreamFromCache(pStream, pCacheResponse, m_pContext->getCache()));
}

Response::Ptr HttpEngine::createUserResponseFromCacheResponse(Response::Ptr pCacheResponse,
        Response::Ptr pNetworkResponse)
{
    Headers::Ptr pCombinedHeaders;
    if (pNetworkResponse) {
        EASYHTTPCPP_LOG_D(Tag, "createUserResponseFromCacheResponse: updateCache");
        pCombinedHeaders = HttpCacheStrategy::combineCacheAndNetworkHeader(pCacheResponse, pNetworkResponse);
        updateCache(pCacheResponse, pNetworkResponse, pCombinedHeaders);
    }

    Response::Builder builder(pCacheResponse);
    builder.setRequest(m_pUserRequest).setPriorResponse(stripBody(m_pPriorResponse))
            .setCacheResponse(stripBody(pCacheResponse)).setBody(createResponseBodyFromCache(pCacheResponse));
    if (pNetworkResponse) {
        builder.setHeaders(pCombinedHeaders).setNetworkResponse(stripBody(pNetworkResponse))
                .setSentRequestSec(pNetworkResponse->getSentRequestSec())
                .setReceivedResponseSec(pNetworkResponse->getReceivedResponseSec());
    }
    return builder.build();
}

Response::Ptr HttpEngine::createUserResponseFromNetworkResponse(Response::Ptr pNetworkResponse)
{
    // create UserResponse from NetworkResponse
    Response::Builder responseBuilder(pNetworkResponse);
    Response::Ptr pUserResponse = responseBuilder.setNetworkResponse(stripBody(pNetworkResponse))
            .setPriorResponse(stripBody(m_pPriorResponse)).build();
    return pUserResponse;
}

Response::Ptr HttpEngine::createUserResponseWithCaching(Response::Ptr pNetworkResponse)
{
    ResponseBody::Ptr pResponseBody = pNetworkResponse->getBody();
    ResponseBodyStreamWithoutCaching* pResponseBodyStream =
            static_cast<ResponseBodyStreamWithoutCaching*> (pResponseBody->getByteStream().get());

    Response::Ptr pStrippedNetworkResponse = stripBody(pNetworkResponse);
    // exchange ResponseBodyStreamWithoutCaching to ResponseBodyStreamWithCaching
    ResponseBodyStream::Ptr pNewResponseBodyStream = pResponseBodyStream->exchangeToResponseBodyStreamWithCaching(
            pStrippedNetworkResponse, m_pContext->getCache());
    ResponseBody::Ptr pNewResponseBody = ResponseBody::create(pResponseBody->getMediaType(),
            pResponseBody->hasContentLength(), pResponseBody->getContentLength(), pNewResponseBodyStream);

    // create UserResponse from NetworkResponse with ResponseBodyStreamWithCaching
    Response::Builder responseBuilder(pStrippedNetworkResponse);
    Response::Ptr pUserResponse = responseBuilder.setBody(pNewResponseBody)
            .setNetworkResponse(pStrippedNetworkResponse).setPriorResponse(stripBody(m_pPriorResponse)).build();
    return pUserResponse;
}

void HttpEngine::removeFromCache(Response::Ptr pNetworkResponse)
{
    // remove from Cache
    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (m_pContext->getCache().get());
    if (pCacheInternal != NULL) {
        pCacheInternal->remove(pNetworkResponse->getRequest());
    }
}

void HttpEngine::updateCache(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse,
        Headers::Ptr pCombinedHeaders)
{
    CacheMetadata::Ptr pCacheMetadata;
    Request::Ptr pRequest = pCacheResponse->getRequest();
    std::string key = HttpUtil::makeCacheKey(pRequest);
    if (key.empty()) {
        EASYHTTPCPP_LOG_D(Tag, "updateCache: can not make key.");
        return;
    }
    HttpCache::Ptr pHttpCache = m_pContext->getCache();
    HttpCacheInternal* pCacheInternal = static_cast<HttpCacheInternal*> (pHttpCache.get());
    CacheManager::Ptr pCacheManager = pCacheInternal->getCacheManager();
    if (!pCacheManager->getMetadata(key, pCacheMetadata)) {
        EASYHTTPCPP_LOG_D(Tag, "updateCache: can not get cache[%s].", key.c_str());
        return;
    }

    HttpCacheMetadata* pHttpCacheMetadata = static_cast<HttpCacheMetadata*> (pCacheMetadata.get());
    pHttpCacheMetadata->setResponseHeaders(pCombinedHeaders);
    pHttpCacheMetadata->setSentRequestAtEpoch(pNetworkResponse->getSentRequestSec());
    pHttpCacheMetadata->setReceivedResponseAtEpoch(pNetworkResponse->getReceivedResponseSec());
    Poco::Timestamp now;
    pHttpCacheMetadata->setCreatedAtEpoch(now.epochTime());

    if (!pCacheManager->putMetadata(key, pCacheMetadata)) {
        EASYHTTPCPP_LOG_D(Tag, "updateCache: can not put cache[%s].", pHttpCacheMetadata->getKey().c_str());
    }
}

bool HttpEngine::isRetryStatusCode(Response::Ptr pResponse)
{
    switch (pResponse->getCode()) {
        case Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT:
        case 308: // Permanent Redirect TODO: Future, to change Once enum is defined.
        case Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY:
        case Poco::Net::HTTPResponse::HTTP_FOUND:
        case Poco::Net::HTTPResponse::HTTP_SEE_OTHER:
        {
            // RFC 2616 10.3 Redirection 3xx
            // except GET, HEAD Method need confirm.
            switch (pResponse->getRequest()->getMethod()) {
                case Request::HttpMethodGet:
                case Request::HttpMethodHead:
                    EASYHTTPCPP_LOG_D(Tag, "isRetryStatusCode: redirect http status [%d]", pResponse->getCode());
                    return true;
                default:
                    EASYHTTPCPP_LOG_D(Tag, "isRetryStatusCode: redirect method is except GET, HEAD. [%s]",
                            HttpUtil::httpMethodToString(pResponse->getRequest()->getMethod()).c_str());
                    return false;
            }
        }
        default:
            return false;
    }
}

} /* namespace easyhttpcpp */
