/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHESTRATEGYLISTENER_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHESTRATEGYLISTENER_H_INCLUDED

namespace easyhttpcpp {
namespace common {

template<class TKey, class TValue>
class CacheStrategyListener {
public:

    virtual ~CacheStrategyListener()
    {
    }

    virtual bool onAdd(const TKey& key, TValue value) = 0;
    virtual bool onUpdate(const TKey& key, TValue value) = 0;
    virtual bool onRemove(const TKey& key) = 0;
    virtual bool onGet(const TKey& key, TValue value) = 0;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHESTRATEGYLISTENER_H_INCLUDED */
