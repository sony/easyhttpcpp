/*
 * Copyright 2019 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_FORWARDINGMOCKSQLITEDATABASE_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_FORWARDINGMOCKSQLITEDATABASE_H_INCLUDED

#include "gmock/gmock.h"

#include "easyhttpcpp/db/ContentValues.h"
#include "easyhttpcpp/db/SqliteDatabase.h"

namespace easyhttpcpp {
namespace testutil {

class ForwardingMockSqliteDatabase : public easyhttpcpp::db::SqliteDatabase {
public:
    typedef Poco::AutoPtr<ForwardingMockSqliteDatabase> Ptr;

    ForwardingMockSqliteDatabase(const std::string& path, easyhttpcpp::db::SqliteDatabase::Ptr pDelegate) :
            easyhttpcpp::db::SqliteDatabase(path), m_pDelegate(pDelegate)
    {
        // By default, all calls are delegated to the delegate object as it is
        ON_CALL(*this, execSql(testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::execSql));
        ON_CALL(*this, query(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
                             testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::query));
        ON_CALL(*this, rawQuery(testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::rawQuery));
        ON_CALL(*this, insert(testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::insert));
        ON_CALL(*this, replace(testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::replace));
        ON_CALL(*this, deleteRows(testing::_, testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::deleteRows));
        ON_CALL(*this, update(testing::_, testing::_, testing::_, testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::update));
        ON_CALL(*this, beginTransaction())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::beginTransaction));
        ON_CALL(*this, endTransaction())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::endTransaction));
        ON_CALL(*this, setTransactionSuccessful())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::setTransactionSuccessful));
        ON_CALL(*this, close())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::close));
        ON_CALL(*this, closeSqliteSession())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::closeSqliteSession));
        ON_CALL(*this, getVersion())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::getVersion));
        ON_CALL(*this, setVersion(testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::setVersion));
        ON_CALL(*this, getAutoVacuum())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::getAutoVacuum));
        ON_CALL(*this, setAutoVacuum(testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::setAutoVacuum));
        ON_CALL(*this, isOpen())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::isOpen));
        ON_CALL(*this, reopen())
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::reopen));
        ON_CALL(*this, setDatabaseCorruptionListener(testing::_))
                .WillByDefault(testing::Invoke(m_pDelegate.get(),
                        &easyhttpcpp::db::SqliteDatabase::setDatabaseCorruptionListener));
    }

    virtual ~ForwardingMockSqliteDatabase()
    {
    }

    MOCK_METHOD1(execSql, void(const std::string& sql));
    MOCK_METHOD9(query, easyhttpcpp::db::SqliteCursor::Ptr(
            const std::string& table, const std::vector<std::string>* columns,
            const std::string* selection, const std::vector<std::string>* selectionArgs, const std::string* groupBy,
            const std::string* having, const std::string* orderBy, const std::string* limit,
            const bool distinct));
    MOCK_METHOD2(rawQuery, easyhttpcpp::db::SqliteCursor::Ptr(
            const std::string& sql, const std::vector<std::string>* selectionArgs));
    MOCK_METHOD2(insert, void(const std::string& table, const easyhttpcpp::db::ContentValues& values));
    MOCK_METHOD2(replace, void(const std::string& table, const easyhttpcpp::db::ContentValues& initialValues));
    MOCK_METHOD3(deleteRows, size_t(const std::string& table, const std::string* whereClause,
            const std::vector<std::string>* whereArgs));
    MOCK_METHOD4(update, size_t(const std::string& table, const easyhttpcpp::db::ContentValues& values,
            const std::string* whereClause, const std::vector<std::string>* whereArgs));
    MOCK_METHOD0(beginTransaction, void());
    MOCK_METHOD0(endTransaction, void());
    MOCK_METHOD0(setTransactionSuccessful, void());
    MOCK_METHOD0(close, void());
    MOCK_METHOD0(closeSqliteSession, void());
    MOCK_METHOD0(getVersion, unsigned int());
    MOCK_METHOD1(setVersion, void(unsigned int version));
    MOCK_METHOD0(getAutoVacuum, AutoVacuum());
    MOCK_METHOD1(setAutoVacuum, void(AutoVacuum autoVacuum));
    MOCK_METHOD0(isOpen, bool());
    MOCK_METHOD0(reopen, void());
    MOCK_METHOD1(setDatabaseCorruptionListener, void(
            easyhttpcpp::db::SqliteDatabaseCorruptionListener::Ptr pListener));

private:
    easyhttpcpp::db::SqliteDatabase::Ptr m_pDelegate;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_FORWARDINGMOCKSQLITEDATABASE_H_INCLUDED */
