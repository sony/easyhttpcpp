/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_PARTIALMOCKSQLITEOPENHELPER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_PARTIALMOCKSQLITEOPENHELPER_H_INCLUDED

#include "gmock/gmock.h"

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "easyhttpcpp/db/SqliteDatabase.h"
#include "easyhttpcpp/db/SqliteOpenHelper.h"

namespace easyhttpcpp {
namespace testutil {

class PartialMockSqliteOpenHelper : public Poco::RefCountedObject,
public easyhttpcpp::db::SqliteOpenHelper {
public:
    typedef Poco::AutoPtr<PartialMockSqliteOpenHelper> Ptr;

    PartialMockSqliteOpenHelper(const Poco::Path& path, unsigned int version) :
    easyhttpcpp::db::SqliteOpenHelper::SqliteOpenHelper(path, version)
    {
    }

    virtual ~PartialMockSqliteOpenHelper()
    {
    }

    MOCK_METHOD1(onCreate, void(easyhttpcpp::db::SqliteDatabase& db));
    MOCK_METHOD1(onConfigure, void(easyhttpcpp::db::SqliteDatabase& db));
    MOCK_METHOD1(onOpen, void(easyhttpcpp::db::SqliteDatabase& db));
    MOCK_METHOD3(onUpgrade, void(easyhttpcpp::db::SqliteDatabase& db, unsigned int oldVersion,
            unsigned int newVersion));
    MOCK_METHOD3(onDowngrade, void(easyhttpcpp::db::SqliteDatabase& db, unsigned int oldVersion,
            unsigned int newVersion));
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_PARTIALMOCKSQLITEOPENHELPER_H_INCLUDED */
