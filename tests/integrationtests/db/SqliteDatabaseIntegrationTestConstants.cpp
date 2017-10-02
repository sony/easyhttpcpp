/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CommonMacros.h"

#include "SqliteDatabaseIntegrationTestConstants.h"

namespace easyhttpcpp {
namespace db {
namespace test {

const char* const SqliteDatabaseIntegrationTestConstants::DatabaseDir = EASYHTTPCPP_STRINGIFY_MACRO(RUNTIME_DATA_ROOT);
const char* const SqliteDatabaseIntegrationTestConstants::DatabaseFileName = "database_test.db";
const char* const SqliteDatabaseIntegrationTestConstants::DatabaseTableName = "table_for_test";

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
