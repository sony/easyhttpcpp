/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/AutoSqliteTransaction.h"
#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteCursor.h"
#include "easyhttpcpp/db/SqliteConflictAlgorithm.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"
#include "PartialMockSqliteOpenHelper.h"
#include "TestDatabaseUtil.h"

using easyhttpcpp::common::StringUtil;
using easyhttpcpp::db::AutoSqliteTransaction;
using easyhttpcpp::db::ContentValues;
using easyhttpcpp::db::SqliteDatabase;
using easyhttpcpp::db::SqlException;
using easyhttpcpp::db::SqliteCursor;
using easyhttpcpp::db::SqliteQueryBuilder;

namespace easyhttpcpp {
namespace testutil {

static const std::string Tag = "TestDatabaseUtil";

TestDatabaseUtil::TestDatabaseUtil()
{
}

bool TestDatabaseUtil::isTableExist(const Poco::Path& databasePath, unsigned int version, const std::string& tableName)
{
    PartialMockSqliteOpenHelper::Ptr pSqliteOpenHelper = new PartialMockSqliteOpenHelper(databasePath, version);
    EXPECT_CALL(*pSqliteOpenHelper, onCreate(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pSqliteOpenHelper, onConfigure(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pSqliteOpenHelper, onOpen(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pSqliteOpenHelper, onUpgrade(testing::_,testing::_,testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(*pSqliteOpenHelper, onDowngrade(testing::_,testing::_,testing::_)).Times(testing::AnyNumber());
    
    bool ret = false;
    SqliteDatabase::Ptr pDb = pSqliteOpenHelper->getReadableDatabase();
    std::string where = StringUtil::format("type='table' and name=\"%s\"", tableName.c_str());
    std::string sql = SqliteQueryBuilder::buildQueryString("sqlite_master", NULL, &where, NULL, NULL, NULL,
            NULL, false);
    SqliteCursor::Ptr pCursor = pDb->rawQuery(sql, NULL);
    if (pCursor->getCount() > 0) {
        ret = true;
    }
    return ret;
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
