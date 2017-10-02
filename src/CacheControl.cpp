/*
 * Copyright 2017 Sony Corporation
 */

#include <limits.h>

#include "Poco/NumberParser.h"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/CacheControl.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"

namespace easyhttpcpp {

static const std::string Tag = "CacheControl";

CacheControl::CacheControl(CacheControl::Builder& builder)
{
    m_maxAgeSec = builder.getMaxAgeSec();
    m_maxStaleSec = builder.getMaxStaleSec();
    m_minFreshSec = builder.getMinFreshSec();
    m_sMaxAgeSec = builder.getSMaxAgeSec();
    m_mustRevalidate = builder.isMustRevalidate();
    m_noCache = builder.isNoCache();
    m_noStore = builder.isNoStore();
    m_noTransform = builder.isNoTransform();
    m_onlyIfCached = builder.isOnlyIfCached();
    m_public = builder.isPublic();
    m_private = builder.isPrivate();
}

CacheControl::~CacheControl()
{
}

CacheControl::Ptr CacheControl::createForceCache()
{
    CacheControl::Builder builder;
    return builder.setOnlyIfCached(true).setMaxStaleSec(LLONG_MAX).build();
}

CacheControl::Ptr CacheControl::createForceNetwork()
{
    CacheControl::Builder builder;
    return builder.setNoCache(true).build();
}

CacheControl::Ptr CacheControl::createFromHeaders(Headers::Ptr pHeaders)
{
    CacheControl::Builder builder;
    if (!pHeaders) {
        return builder.build();
    }
    for (Headers::HeaderMap::ConstIterator it = pHeaders->begin(); it != pHeaders->end(); it++) {
        const std::string& name = it->first;
        const std::string& value = it->second;

        // "Pragma" is HTTP/1.0 backward compatible (RFC2616 14.32 Pragma)
        if (Poco::icompare(name, HttpConstants::HeaderNames::Pragma) == 0) {
            if (Poco::icompare(value, HttpConstants::CacheDirectives::NoCache) == 0) {
                builder.setNoCache(true);
            }
            continue;
        }

        if (Poco::icompare(name, HttpConstants::HeaderNames::CacheControl) != 0) {
            continue;
        }

        // this method do not support colon in quoted string. ex. max-age="1 , 2"

        // divide directive.
        Poco::StringTokenizer directives(value, ",",
                Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);

        // parse directive
        for (size_t directiveIndex = 0; directiveIndex < directives.count(); directiveIndex++) {
            // separate "="
            Poco::StringTokenizer tokens(directives[directiveIndex], "=",
                    Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
            std::string parameter;
            if (tokens.count() > 2) {
                // not supported format for too many "="
                EASYHTTPCPP_LOG_D(Tag, "CacheControl Header format is illegal [%s]", value.c_str());
                continue;
            }
            if (tokens.count() == 2) {
                // exist "="

                parameter = tokens[1];

                // if quoted string, remove double quotation.
                if (parameter.length() > 2 && parameter[0] == '\"' && parameter[parameter.length() - 1] == '\"') {
                    parameter = std::string(parameter, 1, parameter.length() - 2);
                }
            }
            std::string& directive = tokens[0];

            if (Poco::icompare(directive, HttpConstants::CacheDirectives::MaxAge) == 0) {
                builder.setMaxAgeSec(parseNumber(parameter, -1LL));
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::SMaxAge) == 0) {
                builder.setSMaxAgeSec(parseNumber(parameter, -1LL));
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::MaxStale) == 0) {
                builder.setMaxStaleSec(parseNumber(parameter, -1LL));
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::MinFresh) == 0) {
                builder.setMinFreshSec(parseNumber(parameter, -1LL));
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::MustRevalidate) == 0) {
                builder.setMustRevalidate(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::NoCache) == 0) {
                builder.setNoCache(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::NoStore) == 0) {
                builder.setNoStore(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::NoTransform) == 0) {
                builder.setNoTransform(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::OnlyIfCached) == 0) {
                builder.setOnlyIfCached(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::Public) == 0) {
                builder.setPublic(true);
            } else if (Poco::icompare(directive, HttpConstants::CacheDirectives::Private) == 0) {
                builder.setPrivate(true);
            }
        }
    }
    return builder.build();
}

long long CacheControl::getMaxAgeSec() const
{
    return m_maxAgeSec;
}

long long CacheControl::getMaxStaleSec() const
{
    return m_maxStaleSec;
}

long long CacheControl::getMinFreshSec() const
{
    return m_minFreshSec;
}

long long CacheControl::getSMaxAgeSec() const
{
    return m_sMaxAgeSec;
}

bool CacheControl::isMustRevalidate() const
{
    return m_mustRevalidate;
}

bool CacheControl::isNoCache() const
{
    return m_noCache;
}

bool CacheControl::isNoStore() const
{
    return m_noStore;
}

bool CacheControl::isNoTransform() const
{
    return m_noTransform;
}

bool CacheControl::isOnlyIfCached() const
{
    return m_onlyIfCached;
}

bool CacheControl::isPublic() const
{
    return m_public;
}

bool CacheControl::isPrivate() const
{
    return m_private;
}

std::string CacheControl::toString()
{
    return "";
}

long long CacheControl::parseNumber(const std::string& parameter, long long defaultValue)
{
    if (parameter.empty()) {
        return defaultValue;
    } else {
        Poco::Int64 value;
        if (!Poco::NumberParser::tryParse64(parameter, value)) {
            return defaultValue;
        } else {
            return static_cast<long long> (value);
        }
    }
}

CacheControl::Builder::Builder() : m_maxAgeSec(-1), m_maxStaleSec(-1), m_minFreshSec(-1), m_sMaxAgeSec(-1),
        m_mustRevalidate(false), m_noCache(false), m_noStore(false), m_noTransform(false), m_onlyIfCached(false),
        m_public(false), m_private(false)
{
}

CacheControl::Builder::~Builder()
{
}

CacheControl::Ptr CacheControl::Builder::build()
{
    return new CacheControl(*this);
}

CacheControl::Builder& CacheControl::Builder::setMaxAgeSec(long long maxAgeSec)
{
    m_maxAgeSec = maxAgeSec;
    return *this;
}

long long CacheControl::Builder::getMaxAgeSec() const
{
    return m_maxAgeSec;
}

CacheControl::Builder& CacheControl::Builder::setMaxStaleSec(long long maxStaleSec)
{
    m_maxStaleSec = maxStaleSec;
    return *this;
}

long long CacheControl::Builder::getMaxStaleSec() const
{
    return m_maxStaleSec;
}

CacheControl::Builder& CacheControl::Builder::setMinFreshSec(long long minFreshSec)
{
    m_minFreshSec = minFreshSec;
    return *this;
}

long long CacheControl::Builder::getMinFreshSec() const
{
    return m_minFreshSec;
}

CacheControl::Builder& CacheControl::Builder::setSMaxAgeSec(long long sMaxAgeSec)
{
    m_sMaxAgeSec = sMaxAgeSec;
    return *this;
}

long long CacheControl::Builder::getSMaxAgeSec() const
{
    return m_sMaxAgeSec;
}

CacheControl::Builder& CacheControl::Builder::setMustRevalidate(bool mustRevalidate)
{
    m_mustRevalidate = mustRevalidate;
    return *this;
}

bool CacheControl::Builder::isMustRevalidate() const
{
    return m_mustRevalidate;
}

CacheControl::Builder& CacheControl::Builder::setNoCache(bool noCache)
{
    m_noCache = noCache;
    return *this;
}

bool CacheControl::Builder::isNoCache() const
{
    return m_noCache;
}

CacheControl::Builder& CacheControl::Builder::setNoStore(bool noStore)
{
    m_noStore = noStore;
    return *this;
}

bool CacheControl::Builder::isNoStore() const
{
    return m_noStore;
}

CacheControl::Builder& CacheControl::Builder::setNoTransform(bool noTransform)
{
    m_noTransform = noTransform;
    return *this;
}

bool CacheControl::Builder::isNoTransform() const
{
    return m_noTransform;
}

CacheControl::Builder& CacheControl::Builder::setOnlyIfCached(bool onlyIfCached)
{
    m_onlyIfCached = onlyIfCached;
    return *this;
}

bool CacheControl::Builder::isOnlyIfCached() const
{
    return m_onlyIfCached;
}

CacheControl::Builder& CacheControl::Builder::setPublic(bool isPublic)
{
    m_public = isPublic;
    return *this;
}

bool CacheControl::Builder::isPublic() const
{
    return m_public;
}

CacheControl::Builder& CacheControl::Builder::setPrivate(bool isPrivate)
{
    m_private = isPrivate;
    return *this;
}

bool CacheControl::Builder::isPrivate() const
{
    return m_private;
}

} /* namespace easyhttpcpp */
