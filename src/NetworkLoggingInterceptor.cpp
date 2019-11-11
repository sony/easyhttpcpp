/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/HTTPMessage.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpUtil.h"
#include "NetworkLoggingInterceptor.h"

namespace easyhttpcpp {

static const std::string Tag = "NetworkLoggingInterceptor";

NetworkLoggingInterceptor::NetworkLoggingInterceptor()
{
}

NetworkLoggingInterceptor::~NetworkLoggingInterceptor()
{
}

Response::Ptr NetworkLoggingInterceptor::intercept(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
// TODO: NDEBUG is cmake define
#ifdef NDEBUG
    // do not log in production
    return chain.proceed(pRequest);
#else
    std::string protocol = chain.getConnection() ? chain.getConnection()->getProtocol()
            : Poco::Net::HTTPMessage::HTTP_1_1;

    EASYHTTPCPP_LOG_V(Tag, "--> %s %s %s \n%s --> END %s %s", HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(),
            pRequest->getUrl().c_str(), protocol.c_str(), pRequest->getHeaders()->toString().c_str(),
            HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str());

    Poco::Timestamp start;
    try {
        Response::Ptr pResponse = chain.proceed(pRequest);

        EASYHTTPCPP_LOG_V(Tag, "<-- %d %s %s %s (%lld ms)\n%s <-- END %s %s",
                pResponse->getCode(), pResponse->getMessage().c_str(),
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str(),
                // elapsed() return Poco::Timestamp::TimeDiff. Poco::Timestamp::TimeDiff is static cast necessary 
                // because byte size is different between 64 bit build and 32 bit build.
                static_cast<signed long long> (start.elapsed() / 1000), pResponse->getHeaders()->toString().c_str(),
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str());

        return pResponse;
    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_V(Tag, "<-- FAILED %s %s (%lld ms)\nError: %s <-- END %s %s",
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str(),
                // elapsed() return Poco::Timestamp::TimeDiff. Poco::Timestamp::TimeDiff is static cast necessary 
                // because byte size is different between 64 bit build and 32 bit build.
                static_cast<signed long long> (start.elapsed() / 1000), e.getMessage().c_str(),
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str());

        throw;
    }
#endif
}

} /* namespace easyhttpcpp */

