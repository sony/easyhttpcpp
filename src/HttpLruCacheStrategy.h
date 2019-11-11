/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPLRUCACHESTRATEGY_H_INCLUDED
#define EASYHTTPCPP_HTTPLRUCACHESTRATEGY_H_INCLUDED

#include "easyhttpcpp/common/LruCacheByDataSizeStrategy.h"
#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_INTERNAL_API HttpLruCacheStrategy : public easyhttpcpp::common::LruCacheByDataSizeStrategy {
public:
    HttpLruCacheStrategy(size_t maxSize);
    virtual ~HttpLruCacheStrategy();

    virtual bool update(const std::string& key, easyhttpcpp::common::CacheInfoWithDataSize::Ptr pValue);
    virtual bool remove(const std::string& key);
    virtual bool clear(bool mayDeleteIfBusy);

protected:
    virtual bool createRemoveList(LruKeyList& keys, bool mayDeleteIfBusy);
    virtual bool createRemoveLruDataList(LruKeyList& keys, size_t removeSize);
    virtual easyhttpcpp::common::CacheInfoWithDataSize::Ptr newCacheInfo(
            easyhttpcpp::common::CacheInfoWithDataSize::Ptr pCacheInfo);

private:
    HttpLruCacheStrategy();

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPLRUCACHESTRATEGY_H_INCLUDED */
