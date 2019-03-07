/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHEMETADATA_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHEMETADATA_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API CacheMetadata : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<CacheMetadata> Ptr;

    CacheMetadata();
    virtual ~CacheMetadata();

    void setKey(const std::string& key);
    const std::string& getKey() const;

private:
    std::string m_key;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHEMETADATA_H_INCLUDED */
