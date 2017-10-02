/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_COMMONMACROS_H_INCLUDED
#define EASYHTTPCPP_COMMON_COMMONMACROS_H_INCLUDED

namespace easyhttpcpp {
namespace common {

#define EASYHTTPCPP_STRINGIFY_MACRO(macro) EASYHTTPCPP_STRINGIFY_MACRO_ARG(macro)
#define EASYHTTPCPP_STRINGIFY_MACRO_ARG(macro) #macro

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_COMMONMACROS_H_INCLUDED */
