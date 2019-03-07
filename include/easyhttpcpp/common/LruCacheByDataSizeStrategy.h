/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_LRUCACHEBYDATASIZESTRATEGY_H_INCLUDED
#define EASYHTTPCPP_COMMON_LRUCACHEBYDATASIZESTRATEGY_H_INCLUDED

#include <list>
#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/HashMap.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/CacheStrategy.h"
#include "easyhttpcpp/common/CacheInfoWithDataSize.h"
#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API LruCacheByDataSizeStrategy : public CacheStrategy<std::string, CacheInfoWithDataSize::Ptr> {
public:
    typedef Poco::AutoPtr<LruCacheByDataSizeStrategy> Ptr;
    typedef CacheStrategyListener<std::string, CacheInfoWithDataSize::Ptr> LruCacheByDataSizeStrategyListener;

    LruCacheByDataSizeStrategy(size_t maxSize);
    virtual ~LruCacheByDataSizeStrategy();
    virtual void setListener(LruCacheByDataSizeStrategyListener* pListener);
    virtual bool add(const std::string& key, CacheInfoWithDataSize::Ptr pCachInfo);
    virtual bool update(const std::string& key, CacheInfoWithDataSize::Ptr pCachInfo);
    virtual bool remove(const std::string& key);
    virtual CacheInfoWithDataSize::Ptr get(const std::string& key);
    virtual bool clear(bool mayDeleteIfBusy);

    virtual bool makeSpace(size_t requestSize);
    virtual size_t getMaxSize() const;
    virtual size_t getTotalSize() const;
    virtual bool isEmpty() const;

protected:
    typedef std::list<CacheInfoWithDataSize::Ptr> LruList;
    typedef Poco::HashMap<std::string, LruList::iterator> LruListMap;
    typedef std::list<std::string> LruKeyList;

    virtual bool createRemoveList(LruKeyList& keys, bool mayDeleteIfBusy);
    virtual bool createRemoveLruDataList(LruKeyList& keys, size_t removeSize);
    virtual CacheInfoWithDataSize::Ptr newCacheInfo(CacheInfoWithDataSize::Ptr pCacheInfo);

    LruCacheByDataSizeStrategyListener* m_pListener;
    LruList m_lruList;
    LruListMap m_lruListMap;
    size_t m_maxSize;
    size_t m_totalSize;

private:
    LruCacheByDataSizeStrategy();
    bool addOrUpdate(const std::string& key, CacheInfoWithDataSize::Ptr pCacheInfo);
    bool makeSpace(size_t requestSize, const std::string& updatedKey);
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_LRUCACHEBYDATASIZESTRATEGY_H_INCLUDED */
