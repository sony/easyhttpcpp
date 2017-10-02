/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITECONFLICTALGORITHM_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITECONFLICTALGORITHM_H_INCLUDED

namespace easyhttpcpp {
namespace db {

enum SqliteConflictAlgorithm {
    SqliteConflictAlgorithmNone,
    SqliteConflictAlgorithmRollback,
    SqliteConflictAlgorithmAbort,
    SqliteConflictAlgorithmFail,
    SqliteConflictAlgorithmIgnore,
    SqliteConflictAlgorithmReplace
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITECONFLICTALGORITHM_H_INCLUDED */
