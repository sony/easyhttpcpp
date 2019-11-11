/*
 * Copyright 2019 Sony Corporation
 */

#include <string>

#include "HttpCacheDatabaseOpenHelper.h"
#include "HttpInternalConstants.h"

using easyhttpcpp::db::SqliteDatabase;
using easyhttpcpp::db::SqliteOpenHelper;

namespace easyhttpcpp {

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

HttpCacheDatabaseOpenHelper::HttpCacheDatabaseOpenHelper(const Poco::Path& databaseFile) :
        SqliteOpenHelper(databaseFile, HttpInternalConstants::Database::Version)
{
}

HttpCacheDatabaseOpenHelper::~HttpCacheDatabaseOpenHelper()
{
}

void HttpCacheDatabaseOpenHelper::onCreate(SqliteDatabase& db)
{
    std::string sqlCmd = std::string("CREATE TABLE IF NOT EXISTS ") + HttpInternalConstants::Database::TableName +
            " (" + HttpInternalConstants::Database::Key::Id + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            HttpInternalConstants::Database::Key::CacheKey + " TEXT UNIQUE, " +
            HttpInternalConstants::Database::Key::Url + " TEXT, " +
            HttpInternalConstants::Database::Key::Method + " INTEGER, " +
            HttpInternalConstants::Database::Key::StatusCode + " INTEGER, " +
            HttpInternalConstants::Database::Key::StatusMessage + " TEXT, " +
            HttpInternalConstants::Database::Key::ResponseHeaderJson + " TEXT, " +
            HttpInternalConstants::Database::Key::ResponseBodySize + " INTEGER, " +
            HttpInternalConstants::Database::Key::SentRequestAtEpoch + " INTEGER, " +
            HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch + " INTEGER, " +
            HttpInternalConstants::Database::Key::CreatedAtEpoch + " INTEGER, " +
            HttpInternalConstants::Database::Key::LastAccessedAtEpoch + " INTEGER )";
    db.execSql(sqlCmd);
}

void HttpCacheDatabaseOpenHelper::onUpgrade(SqliteDatabase& db, unsigned int oldVersion,
        unsigned int newVersion)
{
}

} /* namespace easyhttpcpp */
