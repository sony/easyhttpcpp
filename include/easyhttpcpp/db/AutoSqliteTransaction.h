/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_AUTOSQLITETRANSACTION_H_INCLUDED
#define EASYHTTPCPP_DB_AUTOSQLITETRANSACTION_H_INCLUDED

#include <string>

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

class AutoSqliteTransaction {
public:
    AutoSqliteTransaction(SqliteDatabase::Ptr pDatabase) : m_pDatabase(pDatabase)
    {
        // throws exception; caller must catch
        if (m_pDatabase) {
            m_pDatabase->beginTransaction();
        }
    }

    virtual ~AutoSqliteTransaction()
    {
        static const std::string Tag = "AutoSqliteTransaction";
        // consumes exception since transaction actually completes during setTransactionSuccessful() call if successful
        try {
            if (m_pDatabase) {
                m_pDatabase->endTransaction();
            }
        } catch (const SqlException& ignored) {
            EASYHTTPCPP_LOG_D(Tag, "Error while endTransaction(). Ignored. Details: %s", ignored.getMessage().c_str());
        }
    }

private:
    SqliteDatabase::Ptr m_pDatabase;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_AUTOSQLITETRANSACTION_H_INCLUDED */
