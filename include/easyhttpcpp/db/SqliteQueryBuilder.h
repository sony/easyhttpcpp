/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITEQUERYBUILDER_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITEQUERYBUILDER_H_INCLUDED

#include <string>
#include <vector>

#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqliteConflictAlgorithm.h"

namespace easyhttpcpp {
namespace db {

class ContentValues;

class SqliteQueryBuilder {
public:
    static std::string buildQueryString(const std::string& table, const std::vector<std::string>* columns,
            const std::string* where, const std::string* groupBy, const std::string* having, const std::string* orderBy,
            const std::string* limit, const bool distinct);

    static std::string buildInsertString(const std::string& table, const ContentValues& values,
            SqliteConflictAlgorithm conflictAlgorithm);

    static std::string buildUpdateString(const std::string& table, const ContentValues& values,
            const std::string* whereClause, SqliteConflictAlgorithm conflictAlgorithm);

    static std::string buildDeleteString(const std::string& table, const std::string* whereClause);
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITEQURERYBUILDER_H_INCLUDED */
