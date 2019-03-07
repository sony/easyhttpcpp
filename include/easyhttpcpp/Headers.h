/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_HEADERS_H_INCLUDED
#define EASYHTTPCPP_HEADERS_H_INCLUDED

#include "Poco/AutoPtr.h"
#include "Poco/ListMap.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/HttpExports.h"

namespace easyhttpcpp {

/**
 * @brief A Headers collect request and response header.
 */
class EASYHTTPCPP_HTTP_API Headers : public Poco::RefCountedObject {
public:
    typedef Poco::AutoPtr<Headers> Ptr;
    typedef Poco::ListMap<std::string, std::string> HeaderMap;

    /**
     * 
     */
    Headers();

    Headers(const Headers& original);

    /**
     * 
     */
    virtual ~Headers();

    Headers& operator=(const Headers& original);

    /**
     * @brief Add Header element.
     * @param name header name
     * @param value header value
     * @exception HttpIllegalArgumentException
     */
    virtual void add(const std::string& name, const std::string& value);

    /**
     * @brief Set Header element.
     * @param name header name
     * @param value header value
     * @exception HttpIllegalArgumentException
     */
    virtual void set(const std::string& name, const std::string& value);

    /**
     * @brief Get header value
     * @param name name to find
     * @param defaultValue if name is found, default value is returned.
     * @return value
     */
    virtual const std::string& getValue(const std::string& name, const std::string& defaultValue) const;

    /**
     * @brief Check header
     * @param name name to check
     * @return if exist true.
     */
    virtual bool has(const std::string& name) const;

    /**
     * @brief Get header element count.
     * @return element count.
     */
    virtual size_t getSize() const;

    /**
     * @brief Check if the header has no elements.
     * @return true if the header is empty, false otherwise
     */
    virtual bool empty() const;

    /**
     * @brief Begin Iterator
     * @return iterator
     */
    virtual HeaderMap::ConstIterator begin() const;

    /**
     * @brief End Iterator
     * @return iterator
     */
    virtual HeaderMap::ConstIterator end() const;

    /**
     * @brief Get header by string.
     * @return string
     */
    virtual std::string toString();

private:
    HeaderMap m_headerItems;

};

} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_HEADERS_H_INCLUDED */
