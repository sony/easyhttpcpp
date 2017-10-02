/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"

#include "Poco/StringTokenizer.h"

#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqliteConflictAlgorithm.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {
namespace test {

class SqliteQueryBuilderUnitTest : public testing::Test {
protected:

    SqliteQueryBuilderUnitTest()
    {
    }

    virtual ~SqliteQueryBuilderUnitTest()
    {
    }
};

class MockContentValues : public ContentValues {
public:

    MockContentValues() : ContentValues()
    {

    }

    virtual ~MockContentValues()
    {
    }

    std::string getStringValue(const std::string& key) const
    {
        throw SqlIllegalArgumentException("IllegalArgument exception for test");
    }

};

class QueryStringParam {
public:
    const char* m_pTable;
    bool m_distinct;
    const char* m_pColumns;
    const char* m_pWhere;
    const char* m_pGroupBy;
    const char* m_pHaving;
    const char* m_pOrderBy;
    const char* m_pLimit;
    const char* m_pExpected;
};

static const char* expectedQueryResult[] = {
    "SELECT DISTINCT * FROM TABLE_A",
    "SELECT * FROM TABLE_B",
    "SELECT A, B, C FROM TABLE_C",
    "SELECT DISTINCT B, C FROM TABLE_D WHERE Name = CSX",
    "SELECT * FROM TABLE_E WHERE Name = CSX GROUP BY Company",
    "SELECT DISTINCT A, B, C FROM TABLE_F GROUP BY Company HAVING AVG(B) = 100",
    "SELECT A, B, C FROM TABLE_G ORDER BY Age DESC",
    "SELECT * FROM TABLE_H ORDER BY Age DESC LIMIT 10"
};

static const QueryStringParam QueryStringParams[] = {
    // table, distinct, columns, where, groupBy, having, orderBy, limit, expected
    {"TABLE_A", true, "", "", "", "", "", "", expectedQueryResult[0]},
    {"TABLE_B", false, "", "", "", "", "", "", expectedQueryResult[1]},
    {"TABLE_C", false, "A,B,C", "", "", "", "", "", expectedQueryResult[2]},
    {"TABLE_D", true, "B,C", "Name = CSX", "", "", "", "", expectedQueryResult[3]},
    {"TABLE_E", false, "", "Name = CSX", "Company", "", "", "", expectedQueryResult[4]},
    {"TABLE_F", true, "A,B,C", "", "Company", "AVG(B) = 100", "", "", expectedQueryResult[5]},
    {"TABLE_G", false, "A,B,C", "", "", "", "Age DESC", "", expectedQueryResult[6]},
    {"TABLE_H", false, "", "", "", "", "Age DESC", "10", expectedQueryResult[7]}
};

class SqliteQueryBuilderBuildQueryStringParameterizedTest : public ::testing::TestWithParam<QueryStringParam> {
};
INSTANTIATE_TEST_CASE_P(SqliteQueryBuilderUnitTest, SqliteQueryBuilderBuildQueryStringParameterizedTest,
        ::testing::ValuesIn(QueryStringParams));

TEST_P(SqliteQueryBuilderBuildQueryStringParameterizedTest, buildQueryString_ReturnsSelectStatement)
{
    // Given: prepare parameters for test
    // split columns, and push back to vectorColumns.
    Poco::StringTokenizer st(GetParam().m_pColumns, ",", Poco::StringTokenizer::TOK_TRIM);

    std::vector<std::string> vectorColumns;
    Poco::StringTokenizer::Iterator it = st.begin();
    while (it != st.end()) {
        vectorColumns.push_back(*it);
        it++;
    }

    std::string where(GetParam().m_pWhere);
    std::string groupBy(GetParam().m_pGroupBy);
    std::string having(GetParam().m_pHaving);
    std::string orderBy(GetParam().m_pOrderBy);
    std::string limit(GetParam().m_pLimit);

    // When: call SqliteQueryBuilder::buildQueryString
    // Then: result of buildQueryString equals to expected string
    EXPECT_STREQ(GetParam().m_pExpected, SqliteQueryBuilder::buildQueryString(GetParam().m_pTable, &vectorColumns,
            &where, &groupBy, &having, &orderBy, &limit, GetParam().m_distinct).c_str());
}

TEST_F(SqliteQueryBuilderUnitTest, buildQueryString_ReturnsSelectStatement_WhenAllOptionalParametersAreNull)
{
    // Given: This test has no no specific conditions

    // When: call SqliteQueryBuilder::buildQueryString with all optional parameters Null
    std::string sql = SqliteQueryBuilder::buildQueryString("TABLE_A", NULL, NULL, NULL, NULL, NULL, NULL, true);

    // Then: buildQueryString returns valid sql statement
    EXPECT_STREQ(expectedQueryResult[0], sql.c_str());
}

TEST_F(SqliteQueryBuilderUnitTest,
        buildQueryString_throwsSqlIllegalArgumentException_WhenGroupByIsNullAndHavingIsNotNull)
{
    // Given: This test has no no specific conditions

    std::string sql;
    try {
        std::string having = "AVG(B) = 100";
        // When: call SqliteQueryBuilder::buildQueryString group by is NULL and having is not NULL
        sql = SqliteQueryBuilder::buildQueryString("TABLE_I", NULL, NULL, NULL, &having, NULL, NULL, false);
        ADD_FAILURE() << "Exception has not thrown in SqliteQueryBuilder::buildQueryString";
    } catch (const SqlIllegalArgumentException& e) {
        // Then: throws SqlIllegalArgumentException and property is as below
        EXPECT_EQ(100200, e.getCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: HAVING clauses are only permitted when using a groupBy clause",
                e.getMessage().c_str());
        EXPECT_TRUE(e.getCause().isNull());
    }
}

TEST_F(SqliteQueryBuilderUnitTest,
        buildQueryString_throwsSqlIllegalArgumentException_WhenLimitUnmatchesRegularExpression)
{
    // Given: This test has no no specific conditions

    std::string sql;
    try {
        // When: call SqliteQueryBuilder::buildQueryString with invalid limit format
        std::string limit = "quiver";
        sql = SqliteQueryBuilder::buildQueryString("TABLE_I", NULL, NULL, NULL, NULL, NULL, &limit, false);
        ADD_FAILURE() << "Exception has not thrown in SqliteQueryBuilder::buildQueryString";
    } catch (const SqlIllegalArgumentException& e) {
        // Then: throws SqlIllegalArgumentException and property is as below
        EXPECT_EQ(100200, e.getCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: invalid LIMIT clauses", e.getMessage().c_str());
        EXPECT_TRUE(e.getCause().isNull());
    }
}

class InsertStringParam {
public:
    const char* m_pTable;
    easyhttpcpp::db::SqliteConflictAlgorithm m_conflictAlgorithm;
    const char* m_pKeys;
    const char* m_pValues;
    const char* m_pExpected;
};

static const char* expectedInsertResult[] = {
    "INSERT INTO TABLE_A () VALUES (NULL)",
    "INSERT OR ROLLBACK INTO TABLE_B () VALUES (NULL)",
    "INSERT OR ABORT INTO TABLE_C () VALUES (NULL)",
    "INSERT OR FAIL INTO TABLE_D () VALUES (NULL)",
    "INSERT OR IGNORE INTO TABLE_E () VALUES (NULL)",
    "INSERT OR REPLACE INTO TABLE_F () VALUES (NULL)",
    "INSERT INTO TABLE_G (name1) VALUES ('value1')",
    "INSERT OR ROLLBACK INTO TABLE_H (name1, name2) VALUES ('value1', 'value2')"
};

static const InsertStringParam InsertStringParams[] = {
    // table, conflict algorithm, keys, values, expected
    {"TABLE_A", SqliteConflictAlgorithmNone, "", "", expectedInsertResult[0]},
    {"TABLE_B", SqliteConflictAlgorithmRollback, "", "", expectedInsertResult[1]},
    {"TABLE_C", SqliteConflictAlgorithmAbort, "", "", expectedInsertResult[2]},
    {"TABLE_D", SqliteConflictAlgorithmFail, "", "", expectedInsertResult[3]},
    {"TABLE_E", SqliteConflictAlgorithmIgnore, "", "", expectedInsertResult[4]},
    {"TABLE_F", SqliteConflictAlgorithmReplace, "", "", expectedInsertResult[5]},
    {"TABLE_G", SqliteConflictAlgorithmNone, "name1", "value1", expectedInsertResult[6]},
    {"TABLE_H", SqliteConflictAlgorithmRollback, "name1,name2", "value1, value2", expectedInsertResult[7]}
};

class SqliteQueryBuilderBuildInsertStringParameterizedTest : public ::testing::TestWithParam<InsertStringParam> {
};
INSTANTIATE_TEST_CASE_P(SqliteQueryBuilderUnitTest, SqliteQueryBuilderBuildInsertStringParameterizedTest,
        ::testing::ValuesIn(InsertStringParams));

TEST_P(SqliteQueryBuilderBuildInsertStringParameterizedTest, buildInsertString_ReturnsInsertStatement)
{
    // Given: prepare parameters for test
    // split keys and values
    Poco::StringTokenizer stKeys(GetParam().m_pKeys, ",", Poco::StringTokenizer::TOK_TRIM);
    Poco::StringTokenizer stValues(GetParam().m_pValues, ",", Poco::StringTokenizer::TOK_TRIM);

    Poco::StringTokenizer::Iterator itKeys = stKeys.begin();
    Poco::StringTokenizer::Iterator itValues = stValues.begin();

    Poco::AutoPtr<ContentValues> pContentValues(new ContentValues);
    while (itKeys != stKeys.end()) {
        pContentValues->put(*itKeys, *itValues);
        itKeys++;
        itValues++;
    }

    // When: call SqliteQueryBuilder::buildInsertString
    // Then: result of buildInsertString equals to expected string.
    EXPECT_STREQ(GetParam().m_pExpected, SqliteQueryBuilder::buildInsertString(GetParam().m_pTable, *pContentValues,
            GetParam().m_conflictAlgorithm).c_str());
}

TEST_F(SqliteQueryBuilderUnitTest,
        buildInsertString_throwsSqlIllegalArgumentException_WhenContentValuesGetStringValueThrowsException)
{
    std::string sql;
    try {
        // Given: prepare parameter
        MockContentValues contentValues;
        contentValues.put("key_1", 1);
        contentValues.put("key_2", "test");

        // When: call SqliteQueryBuilder::buildInsertString, MockContentValues throws exception when getStringValue
        // called
        sql = SqliteQueryBuilder::buildInsertString("table", contentValues, SqliteConflictAlgorithmNone);
        ADD_FAILURE() << "Exception has not thrown in SqliteQueryBuilder::buildInsertString";
    } catch (const SqlIllegalArgumentException& e) {
        // Then: throws SqlIllegalArgumentException and property is as below
        EXPECT_EQ(100200, e.getCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: IllegalArgument exception for test", e.getMessage().c_str());
        EXPECT_TRUE(e.getCause().isNull());
    }
}

class UpdateStringParam {
public:
    const char* m_pTable;
    const char* m_pKeys;
    const char* m_pValues;
    const char* m_pWhereClause;
    easyhttpcpp::db::SqliteConflictAlgorithm m_conflictAlgorithm;
    const char* m_pExpected;
};

static const char* expectedUpdateResult[] = {
    "UPDATE TABLE_A SET name1='value1'",
    "UPDATE OR ROLLBACK TABLE_B SET name1='value1'",
    "UPDATE OR ABORT TABLE_C SET name1='value1'",
    "UPDATE OR FAIL TABLE_D SET name1='value1'",
    "UPDATE OR IGNORE TABLE_E SET name1='value1'",
    "UPDATE OR REPLACE TABLE_F SET name1='value1'",
    "UPDATE TABLE_G SET name1='value1', name2='value2'",
    "UPDATE OR ROLLBACK TABLE_H SET name1='value1', name2='value2' WHERE No = 100"
};

static const UpdateStringParam UpdateStringParams[] = {
    // table, keys, values, whereClause, conflict algorithm, expected
    {"TABLE_A", "name1", "value1", "", SqliteConflictAlgorithmNone, expectedUpdateResult[0]},
    {"TABLE_B", "name1", "value1", "", SqliteConflictAlgorithmRollback, expectedUpdateResult[1]},
    {"TABLE_C", "name1", "value1", "", SqliteConflictAlgorithmAbort, expectedUpdateResult[2]},
    {"TABLE_D", "name1", "value1", "", SqliteConflictAlgorithmFail, expectedUpdateResult[3]},
    {"TABLE_E", "name1", "value1", "", SqliteConflictAlgorithmIgnore, expectedUpdateResult[4]},
    {"TABLE_F", "name1", "value1", "", SqliteConflictAlgorithmReplace, expectedUpdateResult[5]},
    {"TABLE_G", "name1,name2", "value1,value2", "", SqliteConflictAlgorithmNone, expectedUpdateResult[6]},
    {"TABLE_H", "name1,name2", "value1,value2", "No = 100", SqliteConflictAlgorithmRollback, expectedUpdateResult[7]}
};

class SqliteQueryBuilderBuildUpdateStringParameterizedTest : public ::testing::TestWithParam<UpdateStringParam> {
};
INSTANTIATE_TEST_CASE_P(SqliteQueryBuilderUnitTest, SqliteQueryBuilderBuildUpdateStringParameterizedTest,
        ::testing::ValuesIn(UpdateStringParams));

TEST_P(SqliteQueryBuilderBuildUpdateStringParameterizedTest, buildUpdateString_ReturnsUpdateStatement)
{
    // Given: prepare parameters for test
    // split keys and values
    Poco::StringTokenizer stKeys(GetParam().m_pKeys, ",", Poco::StringTokenizer::TOK_TRIM);
    Poco::StringTokenizer stValues(GetParam().m_pValues, ",", Poco::StringTokenizer::TOK_TRIM);

    Poco::StringTokenizer::Iterator itKeys = stKeys.begin();
    Poco::StringTokenizer::Iterator itValues = stValues.begin();

    Poco::AutoPtr<ContentValues> pContentValues(new ContentValues);
    while (itKeys != stKeys.end()) {
        pContentValues->put(*itKeys, *itValues);
        itKeys++;
        itValues++;
    }

    std::string whereClause(GetParam().m_pWhereClause);

    // When: call SqliteQueryBuilder::buildUpdateString
    // Then: result of buildUpdateString equals to expected string
    EXPECT_STREQ(GetParam().m_pExpected, SqliteQueryBuilder::buildUpdateString(GetParam().m_pTable, *pContentValues,
            &whereClause, GetParam().m_conflictAlgorithm).c_str());
}

TEST_F(SqliteQueryBuilderUnitTest, buildUpdateString_ReturnsUpdateStatement_WhenAllOptionalParametersAreNull)
{
    // Given: This test has no no specific conditions

    // When: call SqliteQueryBuilder::buildUpdateString with all optional parameters Null
    ContentValues values;
    values.put("name1", "value1");
    std::string sql = SqliteQueryBuilder::buildUpdateString("TABLE_A", values, NULL, SqliteConflictAlgorithmNone);

    // Then: buildUpdateString returns valid sql statement
    EXPECT_STREQ(expectedUpdateResult[0], sql.c_str());
}

TEST_F(SqliteQueryBuilderUnitTest, buildUpdateString_throwsSqlIllegalArgumentException_WhenContentValuesHasNoElement)
{
    // Given: This test has no no specific conditions

    std::string sql;
    try {
        // When: call SqliteQueryBuilder::buildUpdateString with parameter ContentValues has no element
        Poco::AutoPtr<ContentValues> pContentValues(new ContentValues);
        sql = SqliteQueryBuilder::buildUpdateString("TABLE_A", *pContentValues, NULL, SqliteConflictAlgorithmNone);
        ADD_FAILURE() << "Exception has not thrown in SqliteQueryBuilder::buildUpdateString";
    } catch (const SqlIllegalArgumentException& e) {
        // Then: throws SqlIllegalArgumentException and property is as below
        EXPECT_EQ(100200, e.getCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: ContentValues has no value", e.getMessage().c_str());
        EXPECT_TRUE(e.getCause().isNull());
    }
}

TEST_F(SqliteQueryBuilderUnitTest,
        buildUpdateString_throwsSqlIllegalArgumentException_WhenContentValuesGetStringValueThrowsException)
{
    std::string sql;
    try {
        // Given: prepare parameter
        MockContentValues contentValues;
        contentValues.put("key_1", 1);
        contentValues.put("key_2", "test");

        // When: call SqliteQueryBuilder::buildUpdateString, MockContentValues throws exception when getStringValue
        // called
        sql = SqliteQueryBuilder::buildUpdateString("table", contentValues, NULL, SqliteConflictAlgorithmNone);
        ADD_FAILURE() << "Exception has not thrown in SqliteQueryBuilder::buildUpdateString";
    } catch (const SqlIllegalArgumentException& e) {
        // Then: throws SqlIllegalArgumentException and property is as below
        EXPECT_EQ(100200, e.getCode());
        EXPECT_STREQ("EASYHTTPCPP-ERR-100200: IllegalArgument exception for test", e.getMessage().c_str());
        EXPECT_TRUE(e.getCause().isNull());
    }
}

class DeleteStringParam {
public:
    const char* m_pTable;
    const char* m_pWhereClause;
    const char* m_pExpected;
};

static const DeleteStringParam DeleteStringParams[] = {
    // table, where clause, expected
    {"TABLE_A", "", "DELETE FROM TABLE_A"},
    {"TABLE_B", "No = 100", "DELETE FROM TABLE_B WHERE No = 100"}
};

class SqliteQueryBuilderBuildDeleteStringParameterizedTest : public ::testing::TestWithParam<DeleteStringParam> {
};
INSTANTIATE_TEST_CASE_P(SqliteQueryBuilderUnitTest, SqliteQueryBuilderBuildDeleteStringParameterizedTest,
        ::testing::ValuesIn(DeleteStringParams));

TEST_P(SqliteQueryBuilderBuildDeleteStringParameterizedTest, buildDeleteString_ReturnsDeleteStatement)
{
    // Given: prepare parameters for test
    std::string whereClause(GetParam().m_pWhereClause);

    // When: call SqliteQueryBuilder::buildDeleteString
    // Then: result of buildDeleteString equals to expected string
    EXPECT_STREQ(GetParam().m_pExpected, SqliteQueryBuilder::buildDeleteString(GetParam().m_pTable,
            &whereClause).c_str());
}

TEST_F(SqliteQueryBuilderUnitTest, buildDeleteString_ReturnsSelectStatement_WhenAllOptionalParametersAreNull)
{
    // Given: This test has no no specific conditions

    // When: call SqliteQueryBuilder::buildDeleteString with all optional parameters Null
    std::string sql = SqliteQueryBuilder::buildDeleteString("TABLE_A", NULL);

    // Then: buildDeleteString returns valid sql statement
    EXPECT_STREQ("DELETE FROM TABLE_A", sql.c_str());
}

} /* namespace test */
} /* namespace db */
} /* namespace easyhttpcpp */
