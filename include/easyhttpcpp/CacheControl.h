/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_CACHECONTROL_H_INCLUDED
#define EASYHTTPCPP_CACHECONTROL_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/Headers.h"

namespace easyhttpcpp {

class CacheControl : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<CacheControl> Ptr;

    class Builder;

    /**
     * 
     */
    virtual ~CacheControl();

    /**
     * 
     * @return 
     */
    static CacheControl::Ptr createForceCache();
    static CacheControl::Ptr createForceNetwork();

    static CacheControl::Ptr createFromHeaders(Headers::Ptr pHeaders);

    /**
     * 
     * @return 
     */
    virtual long long getMaxAgeSec() const;

    /**
     * 
     * @return 
     */
    virtual long long getMaxStaleSec() const;

    /**
     * 
     * @return 
     */
    virtual long long getMinFreshSec() const;

    virtual long long getSMaxAgeSec() const;
    /**
     * 
     * @return 
     */
    virtual bool isMustRevalidate() const;

    /**
     * 
     * @return 
     */
    virtual bool isNoCache() const;

    /**
     * 
     * @return 
     */
    virtual bool isNoStore() const;

    /**
     * 
     * @return 
     */
    virtual bool isNoTransform() const;

    /**
     * 
     * @return 
     */
    virtual bool isOnlyIfCached() const;

    virtual bool isPublic() const;
    virtual bool isPrivate() const;

    /**
     * 
     * @return 
     */
    virtual std::string toString();

private:
    CacheControl(Builder& builder);
    static long long parseNumber(const std::string& parameter, long long defaultValue);

    long long m_maxAgeSec;
    long long m_maxStaleSec;
    long long m_minFreshSec;
    long long m_sMaxAgeSec;
    bool m_mustRevalidate;
    bool m_noCache;
    bool m_noStore;
    bool m_noTransform;
    bool m_onlyIfCached;
    bool m_public;
    bool m_private;

public:

    class Builder {
    public:
        /**
         * 
         */
        Builder();

        /**
         * 
         * @return 
         */
        virtual ~Builder();

        CacheControl::Ptr build();

        /**
         * 
         * @param maxAgeSeconds
         * @return 
         */
        Builder& setMaxAgeSec(long long maxAgeSec);
        long long getMaxAgeSec() const;

        /**
         * 
         * @param maxAgeSeconds
         * @return 
         */
        Builder& setMaxStaleSec(long long maxStaleSec);
        long long getMaxStaleSec() const;

        /**
         * 
         * @param maxAgeSeconds
         * @return 
         */
        Builder& setMinFreshSec(long long minFreshSec);
        long long getMinFreshSec() const;

        Builder& setSMaxAgeSec(long long sMaxAgeSec);
        long long getSMaxAgeSec() const;

        Builder& setMustRevalidate(bool mustRevalidate);
        bool isMustRevalidate() const;

        /**
         * 
         * @return 
         */
        Builder& setNoCache(bool noCache);
        bool isNoCache() const;

        /**
         * 
         * @return 
         */
        Builder& setNoStore(bool noStore);
        bool isNoStore() const;

        /**
         * 
         * @return 
         */
        Builder& setNoTransform(bool noTransform);
        bool isNoTransform() const;

        /**
         * 
         * @return 
         */
        Builder& setOnlyIfCached(bool onlyIfCached);
        bool isOnlyIfCached() const;

        Builder& setPublic(bool isPublic);
        bool isPublic() const;

        Builder& setPrivate(bool isPrivate);
        bool isPrivate() const;

    private:
        long long m_maxAgeSec;
        long long m_maxStaleSec;
        long long m_minFreshSec;
        long long m_sMaxAgeSec;
        bool m_mustRevalidate;
        bool m_noCache;
        bool m_noStore;
        bool m_noTransform;
        bool m_onlyIfCached;
        bool m_public;
        bool m_private;
    };
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_CACHECONTROL_H_INCLUDED */
