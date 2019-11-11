/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITEDATABASECORRUPTIONLISTENER_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITEDATABASECORRUPTIONLISTENER_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

/**
 * @class SqliteDatabaseCorruptionListener SqliteDatabaseCorruptionListener.h
 *        "easyhttpcpp/db/SqliteDatabaseCorruptionListener.h"
 * 
 * Listener class to be notified when database is corrupted.
 */
class SqliteDatabaseCorruptionListener : public Poco::RefCountedObject {
public:
    /**
     * A "smart" pointer for SqliteDatabaseCorruptionListener to facilitate reference counting based garbage collection.
     */
    typedef Poco::AutoPtr<SqliteDatabaseCorruptionListener> Ptr;

    virtual ~SqliteDatabaseCorruptionListener()
    {
    }

    /**
     * Called when the database is corrupted.
     * 
     * After this call, the database file may be deleted by the Kit implementation.
     * Copy the database file if necessary.
     * 
     * @param databaseFile Path of databasse file.
     * @param pWhat the SqlException indicating database corruption.
     *          <ul>
     *          <li>SqlDatabaseCorruptException Indicates database corruption.
     *          Check SqlException::getMessage() and SqlException::getCause() for details.</li>
     *          </ul>
     */
    virtual void onDatabaseCorrupt(const std::string& databaseFile, SqlException::Ptr pWhat) = 0;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITEDATABASECORRUPTIONLISTENER_H_INCLUDED */
