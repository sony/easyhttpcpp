/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED
#define EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED

#include "easyhttpcpp/common/CoreException.h"
#include "easyhttpcpp/db/DbExports.h"

namespace easyhttpcpp {
namespace db {

EASYHTTPCPP_DECLARE_EXCEPTION_SUB_GROUP(EASYHTTPCPP_DB_API, SqlException, easyhttpcpp::common::CoreException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_DB_API, SqlExecutionException, SqlException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_DB_API, SqlIllegalArgumentException, SqlException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_DB_API, SqlIllegalStateException, SqlException)

EASYHTTPCPP_DECLARE_EXCEPTION(EASYHTTPCPP_DB_API, SqlDatabaseCorruptException, SqlException)

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLEXCEPTION_H_INCLUDED */
