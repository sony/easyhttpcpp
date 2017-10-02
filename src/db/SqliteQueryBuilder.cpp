/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/RegularExpression.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/StringUtil.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteQueryBuilder.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace db {

static const std::string ConflictAlgorithmStrings[] = {
    "",
    "OR ROLLBACK ",
    "OR ABORT ",
    "OR FAIL ",
    "OR IGNORE ",
    "OR REPLACE "
};

static const std::string LimitPattern = "\\s*\\d+\\s*(,\\s*\\d+\\s*)?";

static const std::string Tag = "SqliteQueryBuilder";

std::string SqliteQueryBuilder::buildQueryString(const std::string& table, const std::vector<std::string>* columns,
        const std::string* where, const std::string* groupBy, const std::string* having, const std::string* orderBy,
        const std::string* limit, const bool distinct)
{
    // create SQL as below
    // SELECT DISTINCT <columns> FROM <Table> WHERE <selection> GROUP BY <groupBy>
    //                                      HAVING <having> ORDER BY <orderBy> LIMIT <limit>

    // parameter check
    if (StringUtil::isNullOrEmpty(groupBy) && !StringUtil::isNullOrEmpty(having)) {
        std::string msg = "HAVING clauses are only permitted when using a groupBy clause";
        EASYHTTPCPP_LOG_D(Tag, msg.c_str());
        throw SqlIllegalArgumentException(msg);
    }

    if (!StringUtil::isNullOrEmpty(limit)) {
        Poco::RegularExpression re(LimitPattern);
        if (!re.match(*limit)) {
            std::string msg = "invalid LIMIT clauses";
            EASYHTTPCPP_LOG_D(Tag, msg.c_str());
            throw SqlIllegalArgumentException(msg);
        }
    }

    std::string queryString;
    queryString.append("SELECT ");

    if (distinct) {
        queryString.append("DISTINCT ");
    }

    if ((!columns) || (columns && columns->size() == 0)) {
        queryString.append("* ");
    } else {
        //append columns
        for (size_t i = 0; i < columns->size(); i++) {
            std::string column = columns->at(i);

            if (!column.empty()) {
                if (i > 0) {
                    queryString.append(", ");
                }
                queryString.append(column);
            }
        }
        queryString.append(" ");
    }

    queryString.append("FROM ");
    queryString.append(table);

    if (!StringUtil::isNullOrEmpty(where)) {
        queryString.append(" WHERE ");
        queryString.append(*where);
    }
    if (!StringUtil::isNullOrEmpty(groupBy)) {
        queryString.append(" GROUP BY ");
        queryString.append(*groupBy);
    }
    if (!StringUtil::isNullOrEmpty(having)) {
        queryString.append(" HAVING ");
        queryString.append(*having);
    }
    if (!StringUtil::isNullOrEmpty(orderBy)) {
        queryString.append(" ORDER BY ");
        queryString.append(*orderBy);
    }
    if (!StringUtil::isNullOrEmpty(limit)) {
        queryString.append(" LIMIT ");
        queryString.append(*limit);
    }

    return queryString;
}

std::string SqliteQueryBuilder::buildInsertString(const std::string& table, const ContentValues& values,
        SqliteConflictAlgorithm conflictAlgorithm)
{
    // create SQL as below
    // INSERT INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')
    // INSERT OR ROLLBACK INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')
    // INSERT OR ABORT INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')
    // INSERT OR FAIL INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')
    // INSERT OR IGNORE INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')
    // INSERT OR REPLACE INTO <Table> (<name1>, <name2>) VALUES('<value1>', '<value2>')

    std::string queryString;
    queryString.append("INSERT ");

    //conflict algorithm
    queryString.append(ConflictAlgorithmStrings[conflictAlgorithm]);

    queryString.append("INTO ");
    queryString.append(table);

    std::vector<std::string> contentValueKeys;
    values.getKeys(contentValueKeys);
    if (contentValueKeys.size() > 0) {
        std::string namesString;
        std::string valuesString;
        size_t size = contentValueKeys.size();
        for (size_t i = 0; i < size; i++) {
            std::string contentValueString = values.getStringValue(contentValueKeys.at(i));
            if (i > 0) {
                namesString.append(", ");
                valuesString.append(", ");
            }
            namesString.append(contentValueKeys.at(i));
            valuesString.append("'");
            valuesString.append(contentValueString);
            valuesString.append("'");
        }
        queryString.append(" (");
        queryString.append(namesString);
        queryString.append(") ");
        queryString.append("VALUES ");
        queryString.append("(");
        queryString.append(valuesString);
        queryString.append(")");
    } else {
        // INSERT conflict algorithm INTO table () VALUES (NULL)
        queryString.append(" () VALUES (NULL)");
    }

    return queryString;
}

std::string SqliteQueryBuilder::buildUpdateString(const std::string& table, const ContentValues& values,
        const std::string* whereClause, SqliteConflictAlgorithm conflictAlgorithm)
{
    // create SQL as below
    // UPDATE <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>
    // UPDATE OR ROLLBACK <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>
    // UPDATE OR ABORT <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>
    // UPDATE OR FAIL <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>
    // UPDATE OR IGNORE <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>
    // UPDATE OR REPLACE <Table> SET <name1>='<value1>', <name2>='<value2>' WHERE <whereClause>

    std::vector<std::string> contentValueKeys;
    values.getKeys(contentValueKeys);
    if (contentValueKeys.size() <= 0) {
        std::string msg = "ContentValues has no value";
        EASYHTTPCPP_LOG_D(Tag, msg.c_str());
        throw SqlIllegalArgumentException(msg);
    }

    std::string queryString;
    queryString.append("UPDATE ");

    //conflict algorithm
    queryString.append(ConflictAlgorithmStrings[conflictAlgorithm]);
    // table
    queryString.append(table);

    // SET
    queryString.append(" SET ");
    std::string namesString;
    std::string valuesString;
    size_t size = contentValueKeys.size();
    for (size_t i = 0; i < size; i++) {
        if (i > 0) {
            queryString.append(", ");
        }
        std::string contentValueString = values.getStringValue(contentValueKeys.at(i));
        queryString.append(contentValueKeys.at(i));
        queryString.append("='");
        queryString.append(contentValueString);
        queryString.append("'");
    }

    if (!StringUtil::isNullOrEmpty(whereClause)) {
        queryString.append(" WHERE ");
        queryString.append(*whereClause);
    }

    return queryString;
}

std::string SqliteQueryBuilder::buildDeleteString(const std::string& table, const std::string* whereClause)
{
    std::string queryString;

    queryString.append("DELETE FROM ");
    queryString.append(table);

    if (!StringUtil::isNullOrEmpty(whereClause)) {
        queryString.append(" WHERE ");
        queryString.append(*whereClause);
    }

    return queryString;
}

} /* namespace db */
} /* namespace easyhttpcpp */
