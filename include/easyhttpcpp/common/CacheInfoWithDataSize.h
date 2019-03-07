/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_CACHEINFOWITHDATASIZE_H_INCLUDED
#define EASYHTTPCPP_COMMON_CACHEINFOWITHDATASIZE_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/common/CommonExports.h"

namespace easyhttpcpp {
namespace common {

class EASYHTTPCPP_COMMON_API CacheInfoWithDataSize : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<CacheInfoWithDataSize> Ptr;

    CacheInfoWithDataSize(const std::string& key, size_t dataSize = 0);
    CacheInfoWithDataSize(const CacheInfoWithDataSize& original);
    virtual ~CacheInfoWithDataSize();

    CacheInfoWithDataSize& operator=(const CacheInfoWithDataSize& original);
    bool operator==(const CacheInfoWithDataSize& target) const;

    void setKey(const std::string& key);
    const std::string& getKey() const;
    void setDataSize(size_t dataSize);
    size_t getDataSize() const;

protected:
    void copyFrom(const CacheInfoWithDataSize& original);

private:
    std::string m_key;
    size_t m_dataSize;
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_CACHEINFOWITHDATASIZE_H_INCLUDED */
