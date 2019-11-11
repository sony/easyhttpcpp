/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/common/FileUtil.h"
#include "easyhttpcpp/db/AutoSqliteCursor.h"
#include "easyhttpcpp/db/AutoSqliteDatabase.h"
#include "easyhttpcpp/db/AutoSqliteTransaction.h"
#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpCacheDatabase.h"
#include "HttpCacheMetadata.h"
#include "HttpCacheEnumerationListener.h"
#include "HttpInternalConstants.h"
#include "HttpUtil.h"

using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheMetadata;
using easyhttpcpp::common::FileUtil;
using easyhttpcpp::db::AutoSqliteCursor;
using easyhttpcpp::db::AutoSqliteDatabase;
using easyhttpcpp::db::AutoSqliteTransaction;
using easyhttpcpp::db::ContentValues;
using easyhttpcpp::db::SqlException;
using easyhttpcpp::db::SqliteCursor;
using easyhttpcpp::db::SqliteDatabase;
using easyhttpcpp::db::SqliteOpenHelper;

namespace easyhttpcpp {

static const std::string Tag = "HttpCacheDatabase";

HttpCacheDatabase::HttpCacheDatabase(HttpCacheDatabaseOpenHelper::Ptr pOpenHelper) : m_pOpenHelper(pOpenHelper)
{
}

HttpCacheDatabase::~HttpCacheDatabase()
{
}

HttpCacheMetadata::Ptr HttpCacheDatabase::getMetadata(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        std::vector<std::string> columns;
        columns.push_back(HttpInternalConstants::Database::Key::Url);
        columns.push_back(HttpInternalConstants::Database::Key::Method);
        columns.push_back(HttpInternalConstants::Database::Key::StatusCode);
        columns.push_back(HttpInternalConstants::Database::Key::StatusMessage);
        columns.push_back(HttpInternalConstants::Database::Key::ResponseHeaderJson);
        columns.push_back(HttpInternalConstants::Database::Key::ResponseBodySize);
        columns.push_back(HttpInternalConstants::Database::Key::SentRequestAtEpoch);
        columns.push_back(HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch);
        columns.push_back(HttpInternalConstants::Database::Key::CreatedAtEpoch);

        std::string whereClause = std::string(HttpInternalConstants::Database::Key::CacheKey) + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        SqliteCursor::Ptr pCursor = pDb->query(HttpInternalConstants::Database::TableName, &columns, &whereClause,
                &whereArgs, NULL, NULL, NULL, NULL);
        AutoSqliteCursor autoSqliteCursor(pCursor);
        if (pCursor->moveToFirst()) {
            HttpCacheMetadata::Ptr pHttpCacheMetadata = new HttpCacheMetadata();
            pHttpCacheMetadata->setKey(key);
            pHttpCacheMetadata->setUrl(pCursor->getString(0));
            pHttpCacheMetadata->setHttpMethod(static_cast<Request::HttpMethod> (pCursor->getInt(1)));
            pHttpCacheMetadata->setStatusCode(pCursor->getInt(2));
            pHttpCacheMetadata->setStatusMessage(pCursor->getString(3));
            pHttpCacheMetadata->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(pCursor->getString(4)));
            pHttpCacheMetadata->setResponseBodySize(pCursor->getUnsignedLongLong(5));
            pHttpCacheMetadata->setSentRequestAtEpoch(pCursor->getUnsignedLongLong(6));
            pHttpCacheMetadata->setReceivedResponseAtEpoch(pCursor->getUnsignedLongLong(7));
            pHttpCacheMetadata->setCreatedAtEpoch(pCursor->getUnsignedLongLong(8));

            // dump
            EASYHTTPCPP_LOG_D(Tag, "getMetadata");
            dumpMetadata(pHttpCacheMetadata);
            return pHttpCacheMetadata;
        } else {
            EASYHTTPCPP_LOG_D(Tag, "getMetadata(): can not get row");
            return NULL;
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while getMetadata(): %s", e.getMessage().c_str());
        throw;
    }
}

bool HttpCacheDatabase::deleteMetadata(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "deleteMetadata() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        std::string whereClause = std::string(HttpInternalConstants::Database::Key::CacheKey) + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        bool deleted = pDb->deleteRows(HttpInternalConstants::Database::TableName, &whereClause, &whereArgs) > 0;

        pDb->setTransactionSuccessful();

        return deleted;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while deleteMetadata(): %s", e.getMessage().c_str());
        // do not set the transaction as successful; database will rollback automatically
        throw;
    }
}

void HttpCacheDatabase::updateMetadata(const std::string& key, HttpCacheMetadata::Ptr pHttpCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        values.put(HttpInternalConstants::Database::Key::CacheKey, pHttpCacheMetadata->getKey());
        values.put(HttpInternalConstants::Database::Key::Url, pHttpCacheMetadata->getUrl());
        values.put(HttpInternalConstants::Database::Key::Method, pHttpCacheMetadata->getHttpMethod());
        values.put(HttpInternalConstants::Database::Key::StatusCode, pHttpCacheMetadata->getStatusCode());
        values.put(HttpInternalConstants::Database::Key::StatusMessage, pHttpCacheMetadata->getStatusMessage());
        values.put(HttpInternalConstants::Database::Key::ResponseHeaderJson,
                HttpUtil::exchangeHeadersToJsonStr(pHttpCacheMetadata->getResponseHeaders()));
        values.put(HttpInternalConstants::Database::Key::ResponseBodySize, pHttpCacheMetadata->getResponseBodySize());
        values.put(HttpInternalConstants::Database::Key::SentRequestAtEpoch,
                pHttpCacheMetadata->getSentRequestAtEpoch());
        values.put(HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch,
                pHttpCacheMetadata->getReceivedResponseAtEpoch());
        values.put(HttpInternalConstants::Database::Key::CreatedAtEpoch, pHttpCacheMetadata->getCreatedAtEpoch());
        Poco::Timestamp now;
        values.put(HttpInternalConstants::Database::Key::LastAccessedAtEpoch, now.epochTime());

        // do an INSERT, and if that INSERT fails because of a conflict,
        // delete the conflicting rows before INSERTing again
        pDb->replace(HttpInternalConstants::Database::TableName, values);
        EASYHTTPCPP_LOG_V(Tag, "New HttpCacheMetadata updateMetadata.");

        pDb->setTransactionSuccessful();

        // dump
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata");
        dumpMetadata(pHttpCacheMetadata);
        EASYHTTPCPP_LOG_D(Tag, "lastAccessedAtEpoch = %s", Poco::DateTimeFormatter::format(now,
                Poco::DateTimeFormat::HTTP_FORMAT).c_str());

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateMetadata(): %s", e.getMessage().c_str());
        throw;
    }
}

bool HttpCacheDatabase::updateLastAccessedSec(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        Poco::Timestamp now;
        values.put(HttpInternalConstants::Database::Key::LastAccessedAtEpoch, now.epochTime());

        EASYHTTPCPP_LOG_D(Tag, "updateLastAccessedSec: lastAccessedAtEpoch = %s", Poco::DateTimeFormatter::format(now,
                Poco::DateTimeFormat::HTTP_FORMAT).c_str());

        std::string whereClause = std::string(HttpInternalConstants::Database::Key::CacheKey) + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        bool update;
        if (pDb->update(HttpInternalConstants::Database::TableName, values, &whereClause, &whereArgs) == 0) {
            // cannot update lastAccessedSec in cache, but ignore it.
            EASYHTTPCPP_LOG_D(Tag, "updateLastAccessedSec : can not update database.");
            update = false;
        } else {
            update = true;
        }
        pDb->setTransactionSuccessful();
        return update;
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateLastAccessedSec(): %s", e.getMessage().c_str());
        throw;
    }
}

bool HttpCacheDatabase::deleteDatabaseFile()
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    return FileUtil::removeFileIfPresent(Poco::File(m_pOpenHelper->getDatabasePath().absolute().toString()));
}

void HttpCacheDatabase::enumerate(HttpCacheEnumerationListener* pListener)
{
    // do not lock in enumerate because deleteMetadata might be called from onEnumerate callback.
    // when call enumerate, exclusive control is done by the caller.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "enumerate() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }

    try {
        SqliteCursor::Ptr pCursor;

        {
            AutoSqliteDatabase autoSqliteDatabase(pDb);

            std::vector<std::string> columns;
            columns.push_back(HttpInternalConstants::Database::Key::CacheKey);
            columns.push_back(HttpInternalConstants::Database::Key::ResponseBodySize);

            // sort by LastAccessedSec.
            std::string orderBy = std::string(HttpInternalConstants::Database::Key::LastAccessedAtEpoch) + " ASC";
            pCursor = pDb->query(HttpInternalConstants::Database::TableName, &columns, NULL, NULL, NULL,
                    NULL, &orderBy, NULL);
        }
        AutoSqliteCursor autoSqliteCursor(pCursor);
        if (pCursor->moveToFirst()) {
            do {
                HttpCacheEnumerationListener::EnumerationParam param;
                param.m_key = pCursor->getString(0);
                param.m_responseBodySize = pCursor->getUnsignedLongLong(1);
                if (!pListener->onEnumerate(param)) {
                    EASYHTTPCPP_LOG_D(Tag, "enumerate: error occurred onEnumerate.");
                    // could not store it in cache, but continue processing because it is cache. 
                }
            } while (pCursor->moveToNext());
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while enumerate(): %s", e.getMessage().c_str());
        throw;
    }
}

void HttpCacheDatabase::closeSqliteSession()
{
    m_pOpenHelper->closeSqliteSession();
}

HttpCacheDatabase::HttpCacheMetadataAll::HttpCacheMetadataAll() : m_lastAccessedAtEpoch(0)
{
}

void HttpCacheDatabase::HttpCacheMetadataAll::setLastAccessedAtEpoch(std::time_t lastAccessedAtEpoch)
{
    m_lastAccessedAtEpoch = lastAccessedAtEpoch;
}

std::time_t HttpCacheDatabase::HttpCacheMetadataAll::getLastAccessedAtEpoch() const
{
    return m_lastAccessedAtEpoch;
}

HttpCacheDatabase::HttpCacheMetadataAll::Ptr HttpCacheDatabase::getMetadataAll(const std::string& key)
{
    // do not use lock because this method is for test.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadataAll() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        std::vector<std::string> columns;
        columns.push_back(HttpInternalConstants::Database::Key::Url);
        columns.push_back(HttpInternalConstants::Database::Key::Method);
        columns.push_back(HttpInternalConstants::Database::Key::StatusCode);
        columns.push_back(HttpInternalConstants::Database::Key::StatusMessage);
        columns.push_back(HttpInternalConstants::Database::Key::ResponseHeaderJson);
        columns.push_back(HttpInternalConstants::Database::Key::ResponseBodySize);
        columns.push_back(HttpInternalConstants::Database::Key::SentRequestAtEpoch);
        columns.push_back(HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch);
        columns.push_back(HttpInternalConstants::Database::Key::CreatedAtEpoch);
        columns.push_back(HttpInternalConstants::Database::Key::LastAccessedAtEpoch);

        std::string whereClause = std::string(HttpInternalConstants::Database::Key::CacheKey) + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        SqliteCursor::Ptr pCursor = pDb->query(HttpInternalConstants::Database::TableName, &columns, &whereClause,
                &whereArgs, NULL, NULL, NULL, NULL);
        AutoSqliteCursor autoSqliteCursor(pCursor);
        if (pCursor->moveToFirst()) {
            HttpCacheDatabase::HttpCacheMetadataAll::Ptr pHttpCacheMetadataAll =
                    new HttpCacheDatabase::HttpCacheMetadataAll();
            pHttpCacheMetadataAll->setKey(key);
            pHttpCacheMetadataAll->setUrl(pCursor->getString(0));
            pHttpCacheMetadataAll->setHttpMethod(static_cast<Request::HttpMethod> (pCursor->getInt(1)));
            pHttpCacheMetadataAll->setStatusCode(pCursor->getInt(2));
            pHttpCacheMetadataAll->setStatusMessage(pCursor->getString(3));
            pHttpCacheMetadataAll->setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(pCursor->getString(4)));
            pHttpCacheMetadataAll->setResponseBodySize(pCursor->getUnsignedLongLong(5));
            pHttpCacheMetadataAll->setSentRequestAtEpoch(pCursor->getUnsignedLongLong(6));
            pHttpCacheMetadataAll->setReceivedResponseAtEpoch(pCursor->getUnsignedLongLong(7));
            pHttpCacheMetadataAll->setCreatedAtEpoch(pCursor->getUnsignedLongLong(8));
            pHttpCacheMetadataAll->setLastAccessedAtEpoch(pCursor->getUnsignedLongLong(9));
            return pHttpCacheMetadataAll;
        } else {
            EASYHTTPCPP_LOG_D(Tag, "getMetadataAll(): can not get row");
            return NULL;
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while getMetadataAll(): %s", e.getMessage().c_str());
        throw;
    }
}

void HttpCacheDatabase::updateMetadataAll(const std::string& key,
        HttpCacheDatabase::HttpCacheMetadataAll::Ptr pHttpCacheMetadataAll)
{
    // do not use lock because this method is for test.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = m_pOpenHelper->getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadataAll() Unable to open database. Error: %s", e.getMessage().c_str());
        throw;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        values.put(HttpInternalConstants::Database::Key::CacheKey, pHttpCacheMetadataAll->getKey());
        values.put(HttpInternalConstants::Database::Key::Url, pHttpCacheMetadataAll->getUrl());
        values.put(HttpInternalConstants::Database::Key::Method, pHttpCacheMetadataAll->getHttpMethod());
        values.put(HttpInternalConstants::Database::Key::StatusCode, pHttpCacheMetadataAll->getStatusCode());
        values.put(HttpInternalConstants::Database::Key::StatusMessage, pHttpCacheMetadataAll->getStatusMessage());
        values.put(HttpInternalConstants::Database::Key::ResponseHeaderJson,
                HttpUtil::exchangeHeadersToJsonStr(pHttpCacheMetadataAll->getResponseHeaders()));
        values.put(HttpInternalConstants::Database::Key::ResponseBodySize,
                pHttpCacheMetadataAll->getResponseBodySize());
        values.put(HttpInternalConstants::Database::Key::SentRequestAtEpoch,
                pHttpCacheMetadataAll->getSentRequestAtEpoch());
        values.put(HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch,
                pHttpCacheMetadataAll->getReceivedResponseAtEpoch());
        values.put(HttpInternalConstants::Database::Key::CreatedAtEpoch, pHttpCacheMetadataAll->getCreatedAtEpoch());
        values.put(HttpInternalConstants::Database::Key::LastAccessedAtEpoch,
                pHttpCacheMetadataAll->getLastAccessedAtEpoch());

        // do an INSERT, and if that INSERT fails because of a conflict,
        // delete the conflicting rows before INSERTing again
        pDb->replace(HttpInternalConstants::Database::TableName, values);
        EASYHTTPCPP_LOG_V(Tag, "New HttpCacheMetadata updateMetadataAll.");

        pDb->setTransactionSuccessful();

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateMetadataAll(): %s", e.getMessage().c_str());
        throw;
    }
}

void HttpCacheDatabase::dumpMetadata(HttpCacheMetadata::Ptr pHttpCacheMetadata)
{
    EASYHTTPCPP_LOG_D(Tag, "url = %s", pHttpCacheMetadata->getUrl().c_str());
    EASYHTTPCPP_LOG_D(Tag, "method = %s", HttpUtil::httpMethodToString(pHttpCacheMetadata->getHttpMethod()).c_str());
    EASYHTTPCPP_LOG_D(Tag, "code = %d", pHttpCacheMetadata->getStatusCode());
    EASYHTTPCPP_LOG_D(Tag, "message = %s", pHttpCacheMetadata->getStatusMessage().c_str());
    EASYHTTPCPP_LOG_D(Tag, "header = %s", HttpUtil::exchangeHeadersToJsonStr(
            pHttpCacheMetadata->getResponseHeaders()).c_str());
    EASYHTTPCPP_LOG_D(Tag, "body size = %zu", pHttpCacheMetadata->getResponseBodySize());
    Poco::Timestamp sentRequestTime = Poco::Timestamp::fromEpochTime(pHttpCacheMetadata->getSentRequestAtEpoch());
    EASYHTTPCPP_LOG_D(Tag, "sentRequestAtEpoch = %s", Poco::DateTimeFormatter::format(sentRequestTime,
            Poco::DateTimeFormat::HTTP_FORMAT).c_str());
    Poco::Timestamp receiveedResponseTime =
            Poco::Timestamp::fromEpochTime(pHttpCacheMetadata->getReceivedResponseAtEpoch());
    EASYHTTPCPP_LOG_D(Tag, "receivedResponseAtEpoch = %s", Poco::DateTimeFormatter::format(receiveedResponseTime,
            Poco::DateTimeFormat::HTTP_FORMAT).c_str());
    Poco::Timestamp createMetadataTime = Poco::Timestamp::fromEpochTime(pHttpCacheMetadata->getCreatedAtEpoch());
    EASYHTTPCPP_LOG_D(Tag, "createAtEpoch = %s", Poco::DateTimeFormatter::format(createMetadataTime,
            Poco::DateTimeFormat::HTTP_FORMAT).c_str());
}

} /* namespace easyhttpcpp */
