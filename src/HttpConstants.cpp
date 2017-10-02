/*
 * Copyright 2017 Sony Corporation
 */

#include "easyhttpcpp/HttpConstants.h"

namespace easyhttpcpp {

const char* const HttpConstants::Caches::DataFileExtention = ".data";
const char* const HttpConstants::Caches::CacheDir = "cache/";
const char* const HttpConstants::Caches::TempDir = "temp/";

const char* const HttpConstants::HeaderNames::Age = "Age";
const char* const HttpConstants::HeaderNames::Authorization = "Authorization";
const char* const HttpConstants::HeaderNames::CacheControl = "Cache-Control";
const char* const HttpConstants::HeaderNames::Connection = "Connection";
const char* const HttpConstants::HeaderNames::ContentEncoding = "Content-Encoding";
const char* const HttpConstants::HeaderNames::ContentType = "Content-Type";
const char* const HttpConstants::HeaderNames::ContentLength = "Content-Length";
const char* const HttpConstants::HeaderNames::Date = "Date";
const char* const HttpConstants::HeaderNames::ETag = "ETag";
const char* const HttpConstants::HeaderNames::Expires = "Expires";
const char* const HttpConstants::HeaderNames::IfModifiedSince = "If-Modified-Since";
const char* const HttpConstants::HeaderNames::IfNoneMatch = "If-None-Match";
const char* const HttpConstants::HeaderNames::KeepAlive = "Keep-Alive";
const char* const HttpConstants::HeaderNames::LastModified = "Last-Modified";
const char* const HttpConstants::HeaderNames::Location = "Location";
const char* const HttpConstants::HeaderNames::Pragma = "Pragma";
const char* const HttpConstants::HeaderNames::ProxyAuthenticate = "Proxy-Authenticate";
const char* const HttpConstants::HeaderNames::ProxyAuthorization = "Proxy-Authorization";
const char* const HttpConstants::HeaderNames::Range = "Range";
const char* const HttpConstants::HeaderNames::Trailers = "Trailers";
const char* const HttpConstants::HeaderNames::TransferEncoding = "Transfer-Encoding";
const char* const HttpConstants::HeaderNames::Te = "TE";
const char* const HttpConstants::HeaderNames::Upgrade = "Upgrade";
const char* const HttpConstants::HeaderNames::UserAgent = "User-Agent";
const char* const HttpConstants::HeaderNames::Warning = "Warning";

const char* const HttpConstants::HeaderValues::Chunked = "chunked";
const char* const HttpConstants::HeaderValues::Close = "close";
const char* const HttpConstants::HeaderValues::HeuristicExpiration = "113 - Heuristic expiration";
const char* const HttpConstants::HeaderValues::ResponseIsStale = "110 - Response is stale";
const char* const HttpConstants::HeaderValues::ApplicationOctetStream = "application/octet-stream";

const char* const HttpConstants::CacheDirectives::MaxAge = "max-age";
const char* const HttpConstants::CacheDirectives::MaxStale = "max-stale";
const char* const HttpConstants::CacheDirectives::MinFresh = "min-fresh";
const char* const HttpConstants::CacheDirectives::MustRevalidate = "must-revalidate";
const char* const HttpConstants::CacheDirectives::NoCache = "no-cache";
const char* const HttpConstants::CacheDirectives::NoStore = "no-store";
const char* const HttpConstants::CacheDirectives::NoTransform = "no-transform";
const char* const HttpConstants::CacheDirectives::OnlyIfCached = "only-if-cached";
const char* const HttpConstants::CacheDirectives::Private = "private";
const char* const HttpConstants::CacheDirectives::Public = "public";
const char* const HttpConstants::CacheDirectives::SMaxAge = "s-maxage";

const char* const HttpConstants::Schemes::Http = "http";
const char* const HttpConstants::Schemes::Https = "https";

const char* const HttpConstants::Database::FileName = "cache_metadata.db";
const char* const HttpConstants::Database::TableName = "cache_metadata";
const unsigned int HttpConstants::Database::Version = 1;


} /* namespace easyhttpcpp */
