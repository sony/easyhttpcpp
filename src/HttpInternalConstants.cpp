/*
 * Copyright 2019 Sony Corporation
 */

#include "HttpInternalConstants.h"

namespace easyhttpcpp {

const char* const HttpInternalConstants::Caches::DataFileExtention = ".data";
const char* const HttpInternalConstants::Caches::CacheDir = "cache/";
const char* const HttpInternalConstants::Caches::TempDir = "temp/";

const char* const HttpInternalConstants::Database::FileName = "cache_metadata.db";
const char* const HttpInternalConstants::Database::TableName = "cache_metadata";
const unsigned int HttpInternalConstants::Database::Version = 1;
const char* const HttpInternalConstants::Database::Key::Id = "id";
const char* const HttpInternalConstants::Database::Key::CacheKey = "cache_key";
const char* const HttpInternalConstants::Database::Key::Url = "url";
const char* const HttpInternalConstants::Database::Key::Method = "method";
const char* const HttpInternalConstants::Database::Key::StatusCode = "status_code";
const char* const HttpInternalConstants::Database::Key::StatusMessage = "status_message";
const char* const HttpInternalConstants::Database::Key::ResponseHeaderJson = "response_header_json";
const char* const HttpInternalConstants::Database::Key::ResponseBodySize = "response_body_size";
const char* const HttpInternalConstants::Database::Key::SentRequestAtEpoch = "sent_request_at_epoch";
const char* const HttpInternalConstants::Database::Key::ReceivedResponseAtEpoch = "received_response_at_epoch";
const char* const HttpInternalConstants::Database::Key::CreatedAtEpoch = "created_at_epoch";
const char* const HttpInternalConstants::Database::Key::LastAccessedAtEpoch = "last_accessed_at_epoch";

const unsigned int HttpInternalConstants::AsyncRequests::DefaultCorePoolSizeOfAsyncThreadPool = 2;
const unsigned int HttpInternalConstants::AsyncRequests::DefaultMaximumPoolSizeOfAsyncThreadPool = 5;

} /* namespace easyhttpcpp */
