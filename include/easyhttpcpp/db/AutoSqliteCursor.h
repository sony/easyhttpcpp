/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_AUTOSQLITECURSOR_H_INCLUDED
#define EASYHTTPCPP_DB_AUTOSQLITECURSOR_H_INCLUDED

#include <string>

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/SqliteCursor.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

class AutoSqliteCursor {
public:
    AutoSqliteCursor(SqliteCursor::Ptr pCursor) : m_pCursor(pCursor)
    {
    }

    virtual ~AutoSqliteCursor()
    {
        static const std::string Tag = "AutoSqliteCursor";
        // consumes exception since sqlite might be able to work even without closing the cursor as well
        try {
            if (m_pCursor) {
                m_pCursor->close();
            }
        } catch (const SqlException& ignored) {
            EASYHTTPCPP_LOG_W(Tag, "Error while closing cursor. Ignored.");
            EASYHTTPCPP_LOG_V(Tag, "Error while closing cursor. Ignored. Details: %s", ignored.getMessage().c_str());
        }
    }

private:
    SqliteCursor::Ptr m_pCursor;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_AUTOSQLITECURSOR_H_INCLUDED */
