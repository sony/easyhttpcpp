/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"
#include "easyhttpcpp/messagedigest/MessageDigestExports.h"

namespace easyhttpcpp {
namespace messagedigest {

/**
 * @class easyhttpcpp::messagedigest::MessageDigestException MessageDigestException.h "easyhttpcpp/messagedigest/MessageDigestException.h"
 * 
 * Defines various exceptions thrown by MessageDigest.
 */
EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(EASYHTTPCPP_MESSAGEDIGEST_API, MessageDigestException, easyhttpcpp::common::CoreException)

/**
 * @class easyhttpcpp::messagedigest::MessageDigestIllegalArgumentException MessageDigestException.h "easyhttpcpp/messagedigest/MessageDigestException.h"
 * 
 * Exception thrown to indicate that a method has been passed an illegal or inappropriate argument.
 */
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_MESSAGEDIGEST_API, MessageDigestIllegalArgumentException, MessageDigestException)
        
/**
 * @class easyhttpcpp::messagedigest::MessageDigestIllegalStateException MessageDigestException.h "easyhttpcpp/messagedigest/MessageDigestException.h"
 * 
 * Exception thrown when a method is invoked at an illegal or inappropriate time or if the provider is
 * not in an appropriate state for the requested operation.
 */
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_MESSAGEDIGEST_API, MessageDigestIllegalStateException, MessageDigestException)
        
/**
 * @class easyhttpcpp::messagedigest::MessageDigestExecutionException MessageDigestException.h "easyhttpcpp/messagedigest/MessageDigestException.h"
 * 
 * Exception thrown when attempting to retrieve the result of a task that aborted by throwing an exception.
 */
EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_MESSAGEDIGEST_API, MessageDigestExecutionException, MessageDigestException)

} /* namespace messagedigest */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_MESSAGEDIGEST_MESSAGEDIGESTEXCEPTION_H_INCLUDED */
