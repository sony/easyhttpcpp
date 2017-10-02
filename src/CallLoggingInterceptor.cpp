/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/HttpException.h"

#include "CallLoggingInterceptor.h"
#include "HttpUtil.h"

namespace easyhttpcpp {

static const std::string Tag = "CallLoggingInterceptor";

CallLoggingInterceptor::CallLoggingInterceptor()
{
}

CallLoggingInterceptor::~CallLoggingInterceptor()
{
}

Response::Ptr CallLoggingInterceptor::intercept(Interceptor::Chain& chain)
{
    Request::Ptr pRequest = chain.getRequest();
// TODO: NDEBUG is cmake define
#ifdef NDEBUG
    // do not log in production
    return chain.proceed(pRequest);
#else
    EASYHTTPCPP_LOG_V(Tag, "Starting %s call to [%s] with headers: \n%s",
            HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str(),
            pRequest->getHeaders()->toString().c_str());

    Poco::Timestamp start;
    try {
        Response::Ptr pResponse = chain.proceed(pRequest);

        EASYHTTPCPP_LOG_V(Tag, "Received response for %s call to [%s] in %lld ms.\n"
                "Response Code: %d, Response message: %s, Response headers: \n%s",
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str(),
                (start.elapsed() / 1000), pResponse->getCode(), pResponse->getMessage().c_str(),
                pResponse->getHeaders()->toString().c_str());

        return pResponse;
    } catch (const HttpException& e) {
        EASYHTTPCPP_LOG_V(Tag, "Failed to receive response for %s call to [%s]. Time taken: %lld ms. \nError: %s",
                HttpUtil::httpMethodToString(pRequest->getMethod()).c_str(), pRequest->getUrl().c_str(),
                (start.elapsed() / 1000), e.getMessage().c_str());

        throw;
    }
#endif
}

} /* namespace easyhttpcpp */

