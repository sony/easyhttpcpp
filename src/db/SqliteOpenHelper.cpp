/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Data/SQLite/Connector.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/db/SqliteOpenHelper.h"

namespace easyhttpcpp {
namespace db {

static const std::string Tag = "SqliteOpenHelper";

SqliteOpenHelper::SqliteOpenHelper(const Poco::Path& path, unsigned int version) : m_initializing(false), m_path(path),
        m_version(version)
{
    if (version < 1) {
        EASYHTTPCPP_LOG_D(Tag, "invalid version %d", version);
        throw SqlIllegalArgumentException(Poco::format("Version must be >=1, was %u", version));
    }
    Poco::Data::SQLite::Connector::registerConnector();
}

SqliteOpenHelper::~SqliteOpenHelper()
{
    // FIXME where should I call this?
    //Poco::Data::SQLite::Connector::unregisterConnector();
}

const Poco::Path& SqliteOpenHelper::getDatabasePath() const
{
    return m_path;
}

unsigned int SqliteOpenHelper::getDatabaseVersion() const
{
    return m_version;
}

void SqliteOpenHelper::close()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    if (m_initializing) {
        std::string msg = "Closed during initialization";
        EASYHTTPCPP_LOG_D(Tag, msg.c_str());
        throw SqlIllegalStateException(msg);
    }

    if (!m_database.isNull() && m_database->isOpen()) {
        m_database->close();
        m_database = NULL;
    }
}

SqliteDatabase::Ptr SqliteOpenHelper::getWritableDatabase()
{
    return getDatabase();
}

SqliteDatabase::Ptr SqliteOpenHelper::getReadableDatabase()
{
    return getDatabase();
}

void SqliteOpenHelper::onConfigure(SqliteDatabase& db)
{
    // NOP
}

void SqliteOpenHelper::onOpen(SqliteDatabase& db)
{
    // NOP
}

void SqliteOpenHelper::onDowngrade(SqliteDatabase& db, unsigned int oldVersion, unsigned int newVersion)
{
    EASYHTTPCPP_LOG_D(Tag, "Can't downgrade database from version %u to %u", oldVersion, newVersion);
}

SqliteDatabase::Ptr SqliteOpenHelper::getDatabase()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    if (!m_database.isNull()) {
        if (!m_database->isOpen()) {
            // Darn! The user closed the database by calling mDatabase.close().
            m_database = NULL;
        } else {
            // The database is already open for business.
            return m_database;
        }
    }

    if (m_initializing) {
        EASYHTTPCPP_LOG_D(Tag, "getDatabase called Recursively");
        throw SqlIllegalStateException("Can't get database for called recursively");
    }

    SqliteDatabase::Ptr db = m_database;

    try {
        m_initializing = true;

        if (!db.isNull()) {
            db->reopen();
        } else {
            db = SqliteDatabase::openOrCreateDatabase(m_path.toString());
        }

        onConfigure(*db);

        unsigned int version = db->getVersion();
        
        if (version == 0) {
            try {
                // Because AutoVacuum needs to be set before table creation,
                // set AutoVacuum immediately after database creation.
                db->setAutoVacuum(SqliteDatabase::AutoVacuumFull);
            } catch (const SqlException& e) {
                // Even if the setting of AutoVacuum fails, the database can be used, so continue processing.
                EASYHTTPCPP_LOG_I(Tag, "Failed to set auto-vacuum status to full.");
                EASYHTTPCPP_LOG_D(Tag, "Failed to set auto-vacuum status to full. Details: %s", e.getMessage().c_str());
            }
        }
        
        if (version != m_version) {
            db->beginTransaction();
            if (version == 0) {
                onCreate(*db);
            } else {
                if (version > m_version) {
                    onDowngrade(*db, version, m_version);
                } else {
                    onUpgrade(*db, version, m_version);
                }
            }
            db->setVersion(m_version);
            db->setTransactionSuccessful();

            db->endTransaction();
        }

        onOpen(*db);
        m_database = db;
        m_initializing = false;
        return db;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getDatabase() failed with error: %s", e.getMessage().c_str());
        
        m_initializing = false;
        throw;
    }
}

} /* namespace db */
} /* namespace easyhttpcpp */
