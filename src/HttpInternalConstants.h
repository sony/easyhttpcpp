/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPINTERNALCONSTANTS_H_INCLUDED
#define EASYHTTPCPP_HTTPINTERNALCONSTANTS_H_INCLUDED

#include <string>

#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

class HttpInternalConstants {
public:

    class EASYHTTPCPP_HTTP_INTERNAL_API Caches {
    public:
        static const char* const DataFileExtention;
        static const char* const CacheDir;
        static const char* const TempDir;
    };

    class EASYHTTPCPP_HTTP_INTERNAL_API Database {
    public:
        static const char* const FileName;
        static const char* const TableName;
        static const unsigned int Version;

        class EASYHTTPCPP_HTTP_INTERNAL_API Key {
        public:
            static const char* const Id;
            static const char* const CacheKey;
            static const char* const Url;
            static const char* const Method;
            static const char* const StatusCode;
            static const char* const StatusMessage;
            static const char* const ResponseHeaderJson;
            static const char* const ResponseBodySize;
            static const char* const SentRequestAtEpoch;
            static const char* const ReceivedResponseAtEpoch;
            static const char* const CreatedAtEpoch;
            static const char* const LastAccessedAtEpoch;
        };
    };

    class EASYHTTPCPP_HTTP_INTERNAL_API AsyncRequests {
    public:
        static const unsigned int DefaultCorePoolSizeOfAsyncThreadPool;
        static const unsigned int DefaultMaximumPoolSizeOfAsyncThreadPool;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPINTERNALCONSTANTS_H_INCLUDED */
