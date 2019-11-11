/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_HTTPCONSTANTS_H_INCLUDED

#include <string>

#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_API HttpConstants {
public:

    class EASYHTTPCPP_HTTP_API HeaderNames {
    public:
        static const char* const Age;
        static const char* const Authorization;
        static const char* const CacheControl;
        static const char* const Connection;
        static const char* const ContentEncoding;
        static const char* const ContentType;
        static const char* const ContentLength;
        static const char* const Date;
        static const char* const ETag;
        static const char* const Expires;
        static const char* const IfModifiedSince;
        static const char* const IfNoneMatch;
        static const char* const KeepAlive;
        static const char* const LastModified;
        static const char* const Location;
        static const char* const Pragma;
        static const char* const ProxyAuthenticate;
        static const char* const ProxyAuthorization;
        static const char* const Range;
        static const char* const Te;
        static const char* const Trailers;
        static const char* const TransferEncoding;
        static const char* const Upgrade;
        static const char* const UserAgent;
        static const char* const Warning;
    };

    class EASYHTTPCPP_HTTP_API HeaderValues {
    public:
        static const char* const Chunked;
        static const char* const Close;
        static const char* const HeuristicExpiration;
        static const char* const ResponseIsStale;
        static const char* const ApplicationOctetStream;
    };

    class EASYHTTPCPP_HTTP_API CacheDirectives {
    public:
        static const char* const MaxAge;
        static const char* const MaxStale;
        static const char* const MinFresh;
        static const char* const MustRevalidate;
        static const char* const NoCache;
        static const char* const NoStore;
        static const char* const NoTransform;
        static const char* const OnlyIfCached;
        static const char* const Private;
        static const char* const Public;
        static const char* const SMaxAge;
    };

    class EASYHTTPCPP_HTTP_API Schemes {
    public:
        static const char* const Http;
        static const char* const Https;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCONSTANTS_H_INCLUDED */
