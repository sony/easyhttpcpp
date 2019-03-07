/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Net/SSLException.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/HttpException.h"

#include "SslContextCreatorImpl.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {

static const std::string Tag = "SslContextCreatorImpl";

Poco::Net::Context::Ptr SslContextCreatorImpl::createContext(EasyHttpContext::Ptr pContext)
{
    const std::string& rootCaDirectory = pContext->getRootCaDirectory();
    const std::string& rootCaFile = pContext->getRootCaFile();
    bool isLoadDefaultCAs = false;
    if (rootCaDirectory.empty() && rootCaFile.empty()) {
        isLoadDefaultCAs = true;
    }
    try {
        // VERIFY_RELAXED is Poco::Net::Context default value.
        // VERIFY_RELAXED is SSL_VERIFY_PEER.
        Poco::Net::Context::VerificationMode verificationMode = Poco::Net::Context::VERIFY_RELAXED;
        int verificationDepth = 9; // 9 is Poco::Net::Context default value.
        Poco::Net::Context::Ptr pPocoContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,
                rootCaDirectory, verificationMode, verificationDepth, isLoadDefaultCAs);
        EASYHTTPCPP_LOG_D(Tag, "create Poco::Net::Context.");
        if (!rootCaFile.empty()) {
            // In Poco::Net:Context, only set either CaFile and CaDirectory.
            // CaFile set here.
            SSL_CTX* pSslCtx = pPocoContext->sslContext();
            if (SSL_CTX_load_verify_locations(pSslCtx, rootCaFile.c_str(), NULL) != 1) {
                EASYHTTPCPP_LOG_D(Tag, "createContext: can not set CaFile. [%s]", rootCaFile.c_str());
                throw HttpSslException(StringUtil::format("can not set CaFile. [%s]", rootCaFile.c_str()));
            }
        }
        pPocoContext->disableProtocols(Poco::Net::Context::PROTO_SSLV2 | Poco::Net::Context::PROTO_SSLV3);
        return pPocoContext;
    } catch (const Poco::Net::SSLException& e) {
        EASYHTTPCPP_LOG_D(Tag, "createContext: SSLException. message=[%s]", e.message().c_str());
        throw HttpSslException(StringUtil::format("SSL error occurred. message=[%s]", e.message().c_str()), e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "createContext: Poco::Exception occurred. message=[%s]", e.message().c_str());
        throw HttpExecutionException(StringUtil::format("sendRequest initialization error occurred. message=[%s]",
                e.message().c_str()), e);
    }
}

} /* namespace easyhttpcpp */
