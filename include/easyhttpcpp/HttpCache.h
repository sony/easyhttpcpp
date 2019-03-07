/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HTTPCACHE_H_INCLUDED
#define EASYHTTPCPP_HTTPCACHE_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/Path.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Path.h"

#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

class EASYHTTPCPP_HTTP_API HttpCache : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<HttpCache> Ptr;

    /**
     * 
     */
    virtual ~HttpCache();

    /**
     * 
     * @param directory
     * @param maxSize
     * @return 
     */
    static HttpCache::Ptr createCache(const Poco::Path& path, size_t maxSize);

    /**
     * 
     * @return 
     */
    virtual const Poco::Path& getPath() const = 0;

    /**
     * 
     */
    virtual void evictAll() = 0;

    /**
     * 
     * @return 
     */
    virtual size_t getMaxSize() const = 0;

    /**
     * 
     * @return 
     */
    virtual size_t getSize() = 0;

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HTTPCACHE_H_INCLUDED */
