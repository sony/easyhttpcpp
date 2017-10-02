/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_BASEEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_COMMON_BASEEXCEPTION_H_INCLUDED

#include <stdexcept>
#include <string>

#include "Poco/SharedPtr.h"

namespace easyhttpcpp {
namespace common {

/**
 * @interface BaseException BaseException.h
 * @brief Base exception class.
 */
class BaseException : public std::exception {
public:
    typedef Poco::SharedPtr<BaseException> Ptr;

    static const std::string DefaultExceptionMessage;

    /**
     * @brief Creates an exception.
     * 
     * @param message
     */
    BaseException(const std::string& message);
    BaseException(const std::string& message, const std::exception& cause);
    BaseException(const BaseException& exception);
    /**
     * @brief Destructor.
     */
    virtual ~BaseException() throw();
    BaseException& operator=(const BaseException& exception);
    /**
     * @brief Returns the exception code if defined.
     * 
     * @return the exception code.
     */
    virtual unsigned int getCode() const;
    /**
     * @brief Gets the message for the exception.
     * 
     * @return the message text.
     */
    virtual const std::string& getMessage() const;

    // Same as getMessage(), but for compatibility with std::exception.
    virtual const char* what() const throw();

    /**
     * @brief Gets the exception.
     * 
     * @return a pointer to the nested exception, or null if no nested exception exists.
     */
    virtual const BaseException::Ptr getCause() const;
    /**
     * @brief Gets the exception group code.
     * 
     * @return the exception group code.
     */
    virtual unsigned int getExceptionGroupCode() const = 0;
    /**
     * @brief Gets the exception subgroup code.
     * 
     * @return the exception subgroup code.
     */
    virtual unsigned int getExceptionSubGroupCode() const = 0;
    /**
     * @brief Gets the exception code.
     * 
     * @return the exception code.
     */
    virtual unsigned int getExceptionCode() const = 0;
    /**
     * @brief Clone exception.
     * 
     * @return this.
     */
    virtual BaseException* clone() const = 0;

    virtual void rethrow() const = 0;

protected:
    std::string m_message;

private:
    BaseException::Ptr m_pCause;
};

/**
 * @brief
 * Macros for quickly declaring and implementing exception classes.
 */
#define EASYHTTPCPP_DECLARE_EXCEPTION_GROUP(CLS)                                                    \
    class CLS : public easyhttpcpp::common::BaseException {                                \
    public:                                                                                 \
        CLS(const std::string& message);                                                    \
        CLS(const std::string& message, const std::exception& cause);                       \
        virtual ~CLS() throw ();                                                            \
        virtual unsigned int getExceptionGroupCode() const;                                 \
    };

#define EASYHTTPCPP_IMPLEMENT_EXCEPTION_GROUP(CLS, GROUPCODE)                                                   \
    CLS::CLS(const std::string& message) : easyhttpcpp::common::BaseException(message)                 \
    {                                                                                                   \
    }                                                                                                   \
    CLS::CLS(const std::string& message, const std::exception& cause) :                                 \
            easyhttpcpp::common::BaseException(message, cause)                                         \
    {                                                                                                   \
    }                                                                                                   \
    CLS::~CLS() throw ()                                                                                \
    {                                                                                                   \
    }                                                                                                   \
    unsigned int CLS::getExceptionGroupCode() const                                                     \
    {                                                                                                   \
        return GROUPCODE;                                                                               \
    }

#define EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(CLS, BASE)                                              \
    class CLS : public BASE {                                                                   \
    public:                                                                                     \
        CLS(const std::string& message);                                                        \
        CLS(const std::string& message, const std::exception& cause);                           \
        virtual ~CLS() throw ();                                                                \
        unsigned int getExceptionSubGroupCode() const;                                          \
    };

#define EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(CLS, BASE, SUBGROUPCODE)                                      \
    CLS::CLS(const std::string& message) : BASE(message)                                                \
    {                                                                                                   \
    }                                                                                                   \
    CLS::CLS(const std::string& message, const std::exception& cause) : BASE(message, cause)            \
    {                                                                                                   \
    }                                                                                                   \
    CLS::~CLS() throw ()                                                                                \
    {                                                                                                   \
    }                                                                                                   \
    unsigned int CLS::getExceptionSubGroupCode() const                                                  \
    {                                                                                                   \
        return SUBGROUPCODE;                                                                            \
    }

#define EASYHTTPCPP_DECLARE_EXCEPTION(CLS, BASE)                                                \
    class CLS : public BASE {                                                           \
    public:                                                                             \
        CLS(const std::string& message);                                                \
        CLS(const std::string& message, const std::exception& cause);                   \
        virtual ~CLS() throw ();                                                        \
        unsigned int getExceptionCode() const;                                          \
        CLS* clone() const;                                                             \
        void rethrow() const;                                                           \
    private:                                                                            \
        virtual std::string createExceptionMessage(const std::string& rawMessage);      \
    };

#define EASYHTTPCPP_IMPLEMENT_EXCEPTION(CLS, BASE, CODE)                                                                    \
    CLS::CLS(const std::string& message) : BASE(message)                                                            \
    {                                                                                                               \
        m_message = createExceptionMessage(message);                                                                \
    }                                                                                                               \
    CLS::CLS(const std::string& message, const std::exception& cause) : BASE(message, cause)                        \
    {                                                                                                               \
        m_message = createExceptionMessage(message);                                                                \
    }                                                                                                               \
    CLS::~CLS() throw ()                                                                                            \
    {                                                                                                               \
    }                                                                                                               \
    unsigned int CLS::getExceptionCode() const                                                                      \
    {                                                                                                               \
        return CODE;                                                                                                \
    }                                                                                                               \
    CLS* CLS::clone() const                                                                                         \
    {                                                                                                               \
        return new CLS(*this);                                                                                      \
    }                                                                                                               \
    void CLS::rethrow() const                                                                                       \
    {                                                                                                               \
        throw *this;                                                                                                \
    }                                                                                                               \
    std::string CLS::createExceptionMessage(const std::string& rawMessage)                                          \
    {                                                                                                               \
        return easyhttpcpp::common::StringUtil::format("%s%u: %s",                                                 \
                easyhttpcpp::common::ExceptionConstants::ErrorCodePrefix.c_str(), getCode(),                       \
                rawMessage.empty() ? easyhttpcpp::common::BaseException::DefaultExceptionMessage.c_str() :         \
                rawMessage.c_str());                                                                                \
    }

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_BASEEXCEPTION_H_INCLUDED */
