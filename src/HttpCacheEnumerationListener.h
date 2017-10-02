/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEENUMERATIONLISTENER_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEENUMERATIONLISTENER_H_INCLUDED

namespace easyhttpcpp {

class HttpCacheEnumerationListener {
public:
    struct EnumerationParam;

    virtual ~HttpCacheEnumerationListener()
    {
    }
    virtual bool onEnumerate(const EnumerationParam& param) = 0;

public:

    struct EnumerationParam {
        std::string m_key;
        size_t m_responseBodySize;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEENUMERATIONLISTENER_H_INCLUDED */
