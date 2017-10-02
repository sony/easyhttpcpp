/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_DB_CURSOR_H_INCLUDED
#define EASYHTTPCPP_DB_CURSOR_H_INCLUDED

#include <string>
#include <vector>

namespace easyhttpcpp {
namespace db {

enum CursorFieldType {
    CursorFieldTypeNull = 0,
    CursorFieldTypeInteger,
    CursorFieldTypeFloat,
    CursorFieldTypeString,
    CursorFieldTypeBlob
};

class Cursor {
public:

    virtual ~Cursor()
    {
    };

    virtual size_t getCount() = 0;
    virtual size_t getColumnCount() = 0;
    virtual size_t getPosition() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual bool isFirst() = 0;
    virtual bool isLast() = 0;
    virtual void close() = 0;
    virtual size_t getColumnIndex(const std::string& columnName) = 0;
    virtual double getDouble(size_t columnIndex) = 0;
    virtual float getFloat(size_t columnIndex) = 0;
    virtual int getInt(size_t columnIndex) = 0;
    virtual long getLong(size_t columnIndex) = 0;
    virtual long long getLongLong(size_t columnIndex) = 0;
    virtual unsigned long long getUnsignedLongLong(size_t columnIndex) = 0;
    virtual std::string getString(size_t columnIndex) = 0;
    virtual CursorFieldType getType(size_t columnIndex) = 0;
    virtual bool isNull(size_t columnIndex) = 0;
    virtual bool move(int offset) = 0;
    virtual bool moveToFirst() = 0;
    virtual bool moveToLast() = 0;
    virtual bool moveToNext() = 0;
    virtual bool moveToPrevious() = 0;
    virtual bool moveToPosition(unsigned int position) = 0;
};

} /* namespace db */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_DB_CURSOR_H_INCLUDED */
