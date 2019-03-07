/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/ExceptionConstants.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

using easyhttpcpp::common::ExceptionConstants;

EASYHTTPCPP_IMPLEMENT_EXCEPTION_SUB_GROUP(SqlException, CoreException, ExceptionConstants::SubGroupCode::Db)

EASYHTTPCPP_IMPLEMENT_EXCEPTION(SqlIllegalArgumentException, SqlException, 0)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(SqlIllegalStateException, SqlException, 1)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(SqlExecutionException, SqlException, 2)
EASYHTTPCPP_IMPLEMENT_EXCEPTION(SqlDatabaseCorruptException, SqlException, 3)

} /* namespace db */
} /* namespace easyhttpcpp */
