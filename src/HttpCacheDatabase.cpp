/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/AutoSqliteCursor.h"
#include "easyhttpcpp/db/AutoSqliteDatabase.h"
#include "easyhttpcpp/db/AutoSqliteTransaction.h"
#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqlException.h"
#include "easyhttpcpp/HttpConstants.h"
#include "easyhttpcpp/HttpException.h"

#include "HttpCacheDatabase.h"
#include "HttpCacheMetadata.h"
#include "HttpCacheEnumerationListener.h"
#include "HttpUtil.h"

using easyhttpcpp::common::Cache;
using easyhttpcpp::common::CacheMetadata;
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
static const std::string KeyId = "id";
static const std::string KeyCacheKey = "cache_key";
static const std::string KeyUrl = "url";
static const std::string KeyMethod = "method";
static const std::string KeyStatusCode = "status_code";
static const std::string KeyStatusMessage = "status_message";
static const std::string KeyResponseHeaderJson = "response_header_json";
static const std::string KeyResponseBodySize = "response_body_size";
static const std::string KeySentRequestAtEpoch = "sent_request_at_epoch";
static const std::string KeyReceivedResponseAtEpoch = "received_response_at_epoch";
static const std::string KeyCreatedAtEpoch = "created_at_epoch";
static const std::string KeyLastAccessedAtEpoch = "last_accessed_at_epoch";

HttpCacheDatabase::HttpCacheDatabase(const Poco::Path& databaseFile) :
        SqliteOpenHelper(databaseFile, HttpConstants::Database::Version)
{
}

HttpCacheDatabase::~HttpCacheDatabase()
{
}

void HttpCacheDatabase::onCreate(SqliteDatabase& db)
{
    std::string sqlCmd = std::string("CREATE TABLE IF NOT EXISTS ") + HttpConstants::Database::TableName +
            " (" + KeyId + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            KeyCacheKey + " TEXT UNIQUE, " +
            KeyUrl + " TEXT, " +
            KeyMethod + " INTEGER, " +
            KeyStatusCode + " INTEGER, " +
            KeyStatusMessage + " TEXT, " +
            KeyResponseHeaderJson + " TEXT, " +
            KeyResponseBodySize + " INTEGER, " +
            KeySentRequestAtEpoch + " INTEGER, " +
            KeyReceivedResponseAtEpoch + " INTEGER, " +
            KeyCreatedAtEpoch + " INTEGER, " +
            KeyLastAccessedAtEpoch + " INTEGER )";
    db.execSql(sqlCmd);
}

void HttpCacheDatabase::onUpgrade(SqliteDatabase& db, unsigned int oldVersion,
        unsigned int newVersion)
{
}

bool HttpCacheDatabase::getMetadata(const std::string& key, HttpCacheMetadata*& pHttpCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    pHttpCacheMetadata = NULL;

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadata() Unable to open database. Error: %s", e.getMessage().c_str());
        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        std::vector<std::string> columns;
        columns.push_back(KeyUrl);
        columns.push_back(KeyMethod);
        columns.push_back(KeyStatusCode);
        columns.push_back(KeyStatusMessage);
        columns.push_back(KeyResponseHeaderJson);
        columns.push_back(KeyResponseBodySize);
        columns.push_back(KeySentRequestAtEpoch);
        columns.push_back(KeyReceivedResponseAtEpoch);
        columns.push_back(KeyCreatedAtEpoch);

        std::string whereClause = KeyCacheKey + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        SqliteCursor::Ptr pCursor = pDb->query(HttpConstants::Database::TableName, &columns, &whereClause, &whereArgs,
                NULL, NULL, NULL, NULL);
        AutoSqliteCursor autoSqliteCursor(pCursor);
        if (pCursor->moveToFirst()) {
            pHttpCacheMetadata = new HttpCacheMetadata();
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
        } else {
            EASYHTTPCPP_LOG_D(Tag, "getMetadata(): can not get row");
            return false;
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while getMetadata(): %s", e.getMessage().c_str());
        return false;
    }
    return true;
}

bool HttpCacheDatabase::deleteMetadata(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "deleteMetadata() Unable to open database. Error: %s", e.getMessage().c_str());

        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    bool deleted = false;
    try {
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        std::string whereClause = KeyCacheKey + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        deleted = pDb->deleteRows(HttpConstants::Database::TableName, &whereClause, &whereArgs) > 0;

        pDb->setTransactionSuccessful();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while deleteMetadata(): %s", e.getMessage().c_str());
        // do not set the transaction as successful; database will rollback automatically
        deleted = false;
    }

    return deleted;
}

bool HttpCacheDatabase::updateMetadata(const std::string& key, HttpCacheMetadata* pHttpCacheMetadata)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata() Unable to open database. Error: %s", e.getMessage().c_str());

        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    bool updated = false;
    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        values.put(KeyCacheKey, pHttpCacheMetadata->getKey());
        values.put(KeyUrl, pHttpCacheMetadata->getUrl());
        values.put(KeyMethod, pHttpCacheMetadata->getHttpMethod());
        values.put(KeyStatusCode, pHttpCacheMetadata->getStatusCode());
        values.put(KeyStatusMessage, pHttpCacheMetadata->getStatusMessage());
        values.put(KeyResponseHeaderJson, HttpUtil::exchangeHeadersToJsonStr(pHttpCacheMetadata->getResponseHeaders()));
        values.put(KeyResponseBodySize, pHttpCacheMetadata->getResponseBodySize());
        values.put(KeySentRequestAtEpoch, pHttpCacheMetadata->getSentRequestAtEpoch());
        values.put(KeyReceivedResponseAtEpoch, pHttpCacheMetadata->getReceivedResponseAtEpoch());
        values.put(KeyCreatedAtEpoch, pHttpCacheMetadata->getCreatedAtEpoch());
        Poco::Timestamp now;
        values.put(KeyLastAccessedAtEpoch, now.epochTime());

        // do an INSERT, and if that INSERT fails because of a conflict,
        // delete the conflicting rows before INSERTing again
        SqliteDatabase::RowId insertedAt = pDb->replace(HttpConstants::Database::TableName, values);
        EASYHTTPCPP_LOG_V(Tag, "New HttpCacheMetadata updateMetadata at row: %lld", insertedAt);

        if (insertedAt > 0) {
            pDb->setTransactionSuccessful();
            updated = true;
        } else {
            // there was some error while replace
            EASYHTTPCPP_LOG_D(Tag, "Database replace error at row: %lld", insertedAt);

            // do not set the transaction as successful; database will rollback automatically
        }

        // dump
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata");
        dumpMetadata(pHttpCacheMetadata);
        EASYHTTPCPP_LOG_D(Tag, "lastAccessedAtEpoch = %s", Poco::DateTimeFormatter::format(now,
                Poco::DateTimeFormat::HTTP_FORMAT).c_str());

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateMetadata(): %s", e.getMessage().c_str());
        updated = false;
    }

    return updated;
}

bool HttpCacheDatabase::updateLastAccessedSec(const std::string& key)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadata() Unable to open database. Error: %s", e.getMessage().c_str());

        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    bool updated = false;
    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        Poco::Timestamp now;
        values.put(KeyLastAccessedAtEpoch, now.epochTime());

        EASYHTTPCPP_LOG_D(Tag, "updateLastAccessedSec: lastAccessedAtEpoch = %s", Poco::DateTimeFormatter::format(now,
                Poco::DateTimeFormat::HTTP_FORMAT).c_str());

        std::string whereClause = KeyCacheKey + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        updated = pDb->update(HttpConstants::Database::TableName, values, &whereClause, &whereArgs) > 0;
        pDb->setTransactionSuccessful();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateLastAccessedSec(): %s", e.getMessage().c_str());
        updated = false;
    }

    return updated;
}

bool HttpCacheDatabase::enumerate(HttpCacheEnumerationListener* pListener)
{
    // do not lock in enumerate because deleteMetadata might be called from onEnumerate callback.
    // when call enumerate, exclusive control is done by the caller.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "enumerate() Unable to open database. Error: %s", e.getMessage().c_str());
        return false;
    }

    try {
        SqliteCursor::Ptr pCursor;

        {
            AutoSqliteDatabase autoSqliteDatabase(pDb);

            std::vector<std::string> columns;
            columns.push_back(KeyCacheKey);
            columns.push_back(KeyResponseBodySize);

            // sort by LastAccessedSec.
            std::string orderBy = KeyLastAccessedAtEpoch + " ASC";
            pCursor = pDb->query(HttpConstants::Database::TableName, &columns, NULL, NULL, NULL,
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
                    return false;
                }
            } while (pCursor->moveToNext());
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while enumerate(): %s", e.getMessage().c_str());
        return false;
    }

    return true;
}

void HttpCacheDatabase::HttpCacheMetadataAll::setLastAccessedAtEpoch(std::time_t lastAccessedAtEpoch)
{
    m_lastAccessedAtEpoch = lastAccessedAtEpoch;
}

std::time_t HttpCacheDatabase::HttpCacheMetadataAll::getLastAccessedAtEpoch() const
{
    return m_lastAccessedAtEpoch;
}

bool HttpCacheDatabase::getMetadataAll(const std::string& key,
        HttpCacheDatabase::HttpCacheMetadataAll& httpCacheMetadataAll)
{
    // do not use lock because this method is for test.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getReadableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getMetadataAll() Unable to open database. Error: %s", e.getMessage().c_str());
        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    try {
        std::vector<std::string> columns;
        columns.push_back(KeyUrl);
        columns.push_back(KeyMethod);
        columns.push_back(KeyStatusCode);
        columns.push_back(KeyStatusMessage);
        columns.push_back(KeyResponseHeaderJson);
        columns.push_back(KeyResponseBodySize);
        columns.push_back(KeySentRequestAtEpoch);
        columns.push_back(KeyReceivedResponseAtEpoch);
        columns.push_back(KeyCreatedAtEpoch);
        columns.push_back(KeyLastAccessedAtEpoch);

        std::string whereClause = KeyCacheKey + "=?";
        std::vector<std::string> whereArgs;
        whereArgs.push_back(key);

        SqliteCursor::Ptr pCursor = pDb->query(HttpConstants::Database::TableName, &columns, &whereClause, &whereArgs,
                NULL, NULL, NULL, NULL);
        AutoSqliteCursor autoSqliteCursor(pCursor);
        if (pCursor->moveToFirst()) {
            httpCacheMetadataAll.setKey(key);
            httpCacheMetadataAll.setUrl(pCursor->getString(0));
            httpCacheMetadataAll.setHttpMethod(static_cast<Request::HttpMethod> (pCursor->getInt(1)));
            httpCacheMetadataAll.setStatusCode(pCursor->getInt(2));
            httpCacheMetadataAll.setStatusMessage(pCursor->getString(3));
            httpCacheMetadataAll.setResponseHeaders(HttpUtil::exchangeJsonStrToHeaders(pCursor->getString(4)));
            httpCacheMetadataAll.setResponseBodySize(pCursor->getUnsignedLongLong(5));
            httpCacheMetadataAll.setSentRequestAtEpoch(pCursor->getUnsignedLongLong(6));
            httpCacheMetadataAll.setReceivedResponseAtEpoch(pCursor->getUnsignedLongLong(7));
            httpCacheMetadataAll.setCreatedAtEpoch(pCursor->getUnsignedLongLong(8));
            httpCacheMetadataAll.setLastAccessedAtEpoch(pCursor->getUnsignedLongLong(9));
        } else {
            EASYHTTPCPP_LOG_D(Tag, "getMetadataAll(): can not get row");
            return false;
        }
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while getMetadataAll(): %s", e.getMessage().c_str());
        return false;
    }
    return true;
}

bool HttpCacheDatabase::updateMetadataAll(const std::string& key,
        HttpCacheDatabase::HttpCacheMetadataAll& httpCacheMetadataAll)
{
    // do not use lock because this method is for test.

    SqliteDatabase::Ptr pDb;
    try {
        pDb = getWritableDatabase();
    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "updateMetadataAll() Unable to open database. Error: %s", e.getMessage().c_str());
        return false;
    }
    AutoSqliteDatabase autoSqliteDatabase(pDb);

    bool updated = false;
    try {
        // always use transactions for speedy and reliable updates
        AutoSqliteTransaction autoSqliteTransaction(pDb);

        ContentValues values;
        values.put(KeyCacheKey, httpCacheMetadataAll.getKey());
        values.put(KeyUrl, httpCacheMetadataAll.getUrl());
        values.put(KeyMethod, httpCacheMetadataAll.getHttpMethod());
        values.put(KeyStatusCode, httpCacheMetadataAll.getStatusCode());
        values.put(KeyStatusMessage, httpCacheMetadataAll.getStatusMessage());
        values.put(KeyResponseHeaderJson,
                HttpUtil::exchangeHeadersToJsonStr(httpCacheMetadataAll.getResponseHeaders()));
        values.put(KeyResponseBodySize, httpCacheMetadataAll.getResponseBodySize());
        values.put(KeySentRequestAtEpoch, httpCacheMetadataAll.getSentRequestAtEpoch());
        values.put(KeyReceivedResponseAtEpoch, httpCacheMetadataAll.getReceivedResponseAtEpoch());
        values.put(KeyCreatedAtEpoch, httpCacheMetadataAll.getCreatedAtEpoch());
        values.put(KeyLastAccessedAtEpoch, httpCacheMetadataAll.getLastAccessedAtEpoch());

        // do an INSERT, and if that INSERT fails because of a conflict,
        // delete the conflicting rows before INSERTing again
        SqliteDatabase::RowId insertedAt = pDb->replace(HttpConstants::Database::TableName, values);
        EASYHTTPCPP_LOG_V(Tag, "New HttpCacheMetadata updateMetadataAll at row: %lld", insertedAt);

        if (insertedAt > 0) {
            pDb->setTransactionSuccessful();
            updated = true;
        } else {
            // there was some error while replace
            EASYHTTPCPP_LOG_D(Tag, "updateMetadataAll: Database replace error at row: %lld", insertedAt);

            // do not set the transaction as successful; database will rollback automatically
        }

    } catch (const SqlException& e) {
        EASYHTTPCPP_LOG_D(Tag, "SQLite error while updateMetadataAll(): %s", e.getMessage().c_str());
        updated = false;
    }
    return updated;
}

void HttpCacheDatabase::dumpMetadata(HttpCacheMetadata* pHttpCacheMetadata)
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
