/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPENGINE_H_INCLUDED
#define EASYHTTPCPP_HTTPENGINE_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/RefCountedObject.h"
#include "Poco/SharedPtr.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPClientSession.h"

#include "easyhttpcpp/HttpExports.h"
#include "easyhttpcpp/Request.h"
#include "easyhttpcpp/Response.h"
#include "easyhttpcpp/ResponseBody.h"
#include "easyhttpcpp/ResponseBodyStream.h"

#include "ConnectionInternal.h"
#include "ConnectionPoolInternal.h"
#include "ConnectionStatusListener.h"
#include "EasyHttpContext.h"
#include "HttpTypedefs.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API HttpEngine : public ConnectionStatusListener, public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpEngine> Ptr;

    HttpEngine(EasyHttpContext::Ptr pContext, Request::Ptr pRequest, Response::Ptr pPriorResponse);
    virtual ~HttpEngine();
    virtual Response::Ptr execute();
    Response::Ptr sendRequestAndReceiveResponseWithRetryByConnection(Request::Ptr pNetworkRequest);
    bool cancel();
    Connection::Ptr getConnection();
    void readAllResponseBodyForCache(Response::Ptr pResponse);
    ConnectionPool::Ptr getConnectionPool();
    bool isConnectionRetried();

    virtual void onIdle(ConnectionInternal* pConnectionInternal, bool& listenerInvalidated);

    static Request::Ptr getRetryRequest(Response::Ptr pResponse);

private:
    typedef Poco::SharedPtr<Poco::Net::HTTPRequest> PocoHttpRequestPtr;
    typedef Poco::SharedPtr<Poco::Net::HTTPResponse> PocoHttpResponsePtr;

    static Response::Ptr stripBody(Response::Ptr pResponse);
    static bool isRetryStatusCode(Response::Ptr pResponse);
    static Request::Ptr makeRetryRequest(Response::Ptr pResponse);

    Response::Ptr sendRequestAndReceiveResponse(Request::Ptr pNetworkRequest, bool forceToCreateConnection,
            bool& connectionReused);
    void sendRequest(PocoHttpClientSessionPtr pPocoHttpClientSession, Request::Ptr pNetworkRequest,
        const Poco::URI& uri, Poco::Timestamp& sentRequestTime);
    Response::Ptr receiveResponse(PocoHttpClientSessionPtr pPocoHttpClientSession, Request::Ptr pNetworkRequest,
            const Poco::URI& uri, Poco::Timestamp& sentRequestTime);

    void checkCacheBeforeSendRequest(Response::Ptr& pUserResponse, Request::Ptr& pNetworkRequest);
    ResponseBody::Ptr createResponseBodyFromCache(Response::Ptr pCacheResponse);
    Response::Ptr createUserResponseFromCacheResponse(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse);
    Response::Ptr createUserResponseFromNetworkResponse(Response::Ptr pNetworkResponse);
    Response::Ptr createUserResponseWithCaching(Response::Ptr pNetworkResponse);
    void removeFromCache(Response::Ptr pNetworkResponse);
    void updateCache(Response::Ptr pCacheResponse, Response::Ptr pNetworkResponse, Headers::Ptr pCombinedHeaders);

    EasyHttpContext::Ptr m_pContext;
    Request::Ptr m_pUserRequest;
    Response::Ptr m_pPriorResponse;
    Response::Ptr m_pCacheResponse;

    ConnectionInternal::Ptr m_pConnectionInternal;
    bool m_cancelled;
    Poco::FastMutex m_connectionMutex;
    ConnectionPoolInternal::Ptr m_pConnectionPoolInternal;
    bool m_connectionRetried;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPENGINE_H_INCLUDED */
