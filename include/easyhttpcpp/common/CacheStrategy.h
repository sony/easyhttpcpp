/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHESTRATEGY_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHESTRATEGY_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/ScopedLock.h"

#include "easyhttpcpp/common/CacheStrategyListener.h"

namespace easyhttpcpp {
namespace common {

// TValue is intended Poco::AutoPtr.

template<class TKey, class TValue>
class CacheStrategy : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<CacheStrategy<TKey, TValue> > Ptr;
    typedef Poco::ScopedLock<CacheStrategy<TKey, TValue> > ScopedLock;

    virtual ~CacheStrategy()
    {
    }

    virtual void setListener(CacheStrategyListener <TKey, TValue>* pListener) = 0;
    virtual bool add(const TKey& key, TValue value) = 0;
    virtual bool update(const TKey& key, TValue value) = 0;
    virtual bool remove(const TKey& key) = 0;
    virtual TValue get(const TKey& key) = 0;
    virtual bool clear(bool mayDeleteIfBusy) = 0;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHESTRATEGY_H_INCLUDED */
