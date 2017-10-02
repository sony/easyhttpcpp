/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"

namespace easyhttpcpp {
namespace db {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(SqlException, easyhttpcpp::common::CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(SqlExecutionException, SqlException)

EASYHTTPCPP_DECLARE_EXCEPTION(SqlIllegalArgumentException, SqlException)

EASYHTTPCPP_DECLARE_EXCEPTION(SqlIllegalStateException, SqlException)

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED */
