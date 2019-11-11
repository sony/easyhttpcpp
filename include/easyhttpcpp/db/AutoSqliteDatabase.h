/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_AUTOSQLITEDATABASE_H_INCLUDED
#define EASYHTTPCPP_DB_AUTOSQLITEDATABASE_H_INCLUDED

#include <string>

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

class AutoSqliteDatabase {
public:
    AutoSqliteDatabase(SqliteDatabase::Ptr pDatabase) : m_pDatabase(pDatabase)
    {
    }

    virtual ~AutoSqliteDatabase()
    {
        static const std::string Tag = "AutoSqliteDatabase";
        // consumes exception since sqlite might be able to work even without closing database as well
        try {
            if (m_pDatabase) {
                m_pDatabase->close();
            }
        } catch (const SqlException& ignored) {
            EASYHTTPCPP_LOG_D(Tag, "Error while closing database. Ignored. Details: %s", ignored.getMessage().c_str());
        }
    }

private:
    SqliteDatabase::Ptr m_pDatabase;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_AUTOSQLITEDATABASE_H_INCLUDED */
