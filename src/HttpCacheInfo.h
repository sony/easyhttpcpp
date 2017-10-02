/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHEINFO_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHEINFO_H_INCLUDED

#include "easyhttpcpp/common/CacheInfoWithDataSize.h"

namespace easyhttpcpp {

class HttpCacheInfo : public easyhttpcpp::common::CacheInfoWithDataSize {
public:
    HttpCacheInfo(const std::string& key, size_t dataSize = 0);
    HttpCacheInfo(const HttpCacheInfo& original);
    virtual ~HttpCacheInfo();

    HttpCacheInfo& operator = (const HttpCacheInfo& original);
    bool operator == (const HttpCacheInfo& target) const;

    void setReservedRemove(bool reservedRemove);
    bool isReservedRemove() const;
    void addDataRef();
    void releaseDataRef();
    unsigned int getDataRefCount() const;

private:
    bool m_reservedRemove;
    unsigned int m_dataRefCount;
};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHEINFO_H_INCLUDED */
