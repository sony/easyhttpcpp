/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_SQLITECURSOR_H_INCLUDED
#define EASYHTTPCPP_DB_SQLITECURSOR_H_INCLUDED

#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/SharedPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Data/RecordSet.h"

#include "easyhttpcpp/db/Cursor.h"
#include "easyhttpcpp/db/DbExports.h"

namespace easyhttpcpp {
namespace db {

class EASYHTTPCPP_DB_API SqliteCursor : public Poco::RefCountedObject, public Cursor {
public:
    typedef Poco::AutoPtr<SqliteCursor> Ptr;

    SqliteCursor(const Poco::Data::Statement& statement);
    virtual ~SqliteCursor();

    virtual size_t getCount();
    virtual size_t getColumnCount();
    virtual size_t getPosition();
    virtual std::vector<std::string> getColumnNames();
    virtual bool isFirst();
    virtual bool isLast();
    virtual void close();
    virtual size_t getColumnIndex(const std::string& columnName);
    virtual double getDouble(size_t columnIndex);
    virtual double optGetDouble(size_t columnIndex, double defaultValue);
    virtual float getFloat(size_t columnIndex);
    virtual float optGetFloat(size_t columnIndex, float defaultValue);
    virtual int getInt(size_t columnIndex);
    virtual int optGetInt(size_t columnIndex, int defaultValue);
    virtual long getLong(size_t columnIndex);
    virtual long optGetLong(size_t columnIndex, long defaultValue);
    virtual long long getLongLong(size_t columnIndex);
    virtual long long optGetLongLong(size_t columnIndex, long long defaultValue);
    virtual unsigned long long getUnsignedLongLong(size_t columnIndex);
    virtual unsigned long long optGetUnsignedLongLong(size_t columnIndex, unsigned long long defaultValue);
    virtual std::string getString(size_t columnIndex);
    virtual std::string optGetString(size_t columnIndex, const std::string& defaultValue);
    virtual CursorFieldType getType(size_t columnIndex);
    virtual bool isNull(size_t columnIndex);
    virtual bool move(int offset);
    virtual bool moveToFirst();
    virtual bool moveToLast();
    virtual bool moveToNext();
    virtual bool moveToPrevious();
    virtual bool moveToPosition(unsigned int position);
private:
    void throwExceptionIfIllegalState(const std::string& func);

    Poco::SharedPtr<Poco::Data::RecordSet> m_pRecordSet;
    size_t m_currentRow;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_SQLITECURSOR_H_INCLUDED */
