/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Exception.h"
#include "Poco/NumberParser.h"

#include "easyhttpcpp/common/CoreLogger.h"
#include "easyhttpcpp/db/SqliteCursor.h"
#include "easyhttpcpp/db/SqlException.h"

namespace easyhttpcpp {
namespace db {

static const std::string Tag = "SqliteCursor";

SqliteCursor::SqliteCursor(const Poco::Data::Statement& statement) : m_currentRow(0)
{
    m_pRecordSet = new Poco::Data::RecordSet(statement);
}

SqliteCursor::~SqliteCursor()
{
    if (m_pRecordSet) {
        m_pRecordSet = NULL;
    }
    m_currentRow = 0;
}

size_t SqliteCursor::getCount()
{
    throwExceptionIfIllegalState(__func__);

    try {
        // TODO: check with rowCount()
        return m_pRecordSet->getTotalRowCount();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getCount failed");
        throw SqlExecutionException("getCount failed", e);
    }
}

size_t SqliteCursor::getColumnCount()
{
    throwExceptionIfIllegalState(__func__);
    try {
        return m_pRecordSet->columnCount();
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getColumnCount failed");
        throw SqlExecutionException("getColumntCount failed", e);
    }
}

size_t SqliteCursor::getPosition()
{
    throwExceptionIfIllegalState(__func__);
    return m_currentRow;
}

std::vector<std::string> SqliteCursor::getColumnNames()
{
    throwExceptionIfIllegalState(__func__);

    try {
        std::vector<std::string> columnNames;
        size_t columnCount = m_pRecordSet->columnCount();
        for (size_t i = 0; i < columnCount; i++) {
            std::string columnName = m_pRecordSet->columnName(i);
            columnNames.push_back(columnName);
        }
        return columnNames;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getColumnNames failed");
        throw SqlExecutionException("getColumnNames failed", e);
    }
}

bool SqliteCursor::isFirst()
{
    throwExceptionIfIllegalState(__func__);
    bool ret = false;
    if (m_currentRow == 0) {
        ret = true;
    }
    return ret;
}

bool SqliteCursor::isLast()
{
    throwExceptionIfIllegalState(__func__);
    bool ret = false;
    if (m_currentRow == m_pRecordSet->getTotalRowCount() - 1) {
        ret = true;
    }
    return ret;
}

void SqliteCursor::close()
{
    throwExceptionIfIllegalState(__func__);
    m_pRecordSet = NULL;
    m_currentRow = 0;
}

size_t SqliteCursor::getColumnIndex(const std::string& columnName)
{
    throwExceptionIfIllegalState(__func__);
    std::vector<std::string> columnNames = getColumnNames();
    size_t namesSize = columnNames.size();
    if (namesSize <= 0) {
        std::string msg = "Database has no columns";
        EASYHTTPCPP_LOG_D(Tag, msg.c_str());
        throw SqlIllegalStateException(msg);
    }
    size_t index = 0;
    bool found = false;
    for (index = 0; index < namesSize; index++) {
        std::string name = columnNames.at(index);
        if (name == columnName) {
            found = true;
            break;
        }
    }
    if (!found) {
        EASYHTTPCPP_LOG_D(Tag, "getColumnIndex not found [%s]", columnName.c_str());
        throw SqlIllegalArgumentException("getColumnIndex failed");
    }
    return index;
}

double SqliteCursor::getDouble(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        double result = 0;
        value.convert(result);
        return result;
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getDouble Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getDouble Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getDouble Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getDouble Failed", e);
    }
}

double SqliteCursor::optGetDouble(size_t columnIndex, double defaultValue)
{
    try {
        return getDouble(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

float SqliteCursor::getFloat(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        float result = 0;
        value.convert(result);
        return result;
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getFloat Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getFloat Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getFloat Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getFloat Failed", e);
    }
}

float SqliteCursor::optGetFloat(size_t columnIndex, float defaultValue)
{
    try {
        return getFloat(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

int SqliteCursor::getInt(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        int result = 0;
        value.convert(result);
        return result;
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getInt Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getInt Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getInt Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getInt Failed", e);
    }
}

int SqliteCursor::optGetInt(size_t columnIndex, int defaultValue)
{
    try {
        return getInt(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

long SqliteCursor::getLong(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        long result = 0;
        value.convert(result);
        return result;
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getLong Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getLong Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getLong Failed columnIndex [%d]", columnIndex);
        throw SqlExecutionException("getLong Failed", e);
    }
}

long SqliteCursor::optGetLong(size_t columnIndex, long defaultValue)
{
    try {
        return getLong(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

long long SqliteCursor::getLongLong(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        std::string stringResult;
        value.convert(stringResult);

        Poco::Int64 result = Poco::NumberParser::parse64(stringResult);
        return static_cast<long long> (result);
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getLongLong Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getLongLong Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getLongLong Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getLongLong Failed", e);
    }
}

long long SqliteCursor::optGetLongLong(size_t columnIndex, long long defaultValue)
{
    try {
        return getLongLong(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

unsigned long long SqliteCursor::getUnsignedLongLong(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        std::string stringResult;
        value.convert(stringResult);

        Poco::UInt64 result = Poco::NumberParser::parseUnsigned64(stringResult);
        return static_cast<unsigned long long> (result);
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getUnsignedLongLong Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getUnsignedLongLong Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getUnsignedLongLong Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getUnsignedLongLong Failed", e);
    }
}

unsigned long long SqliteCursor::optGetUnsignedLongLong(size_t columnIndex, unsigned long long defaultValue)
{
    try {
        return getUnsignedLongLong(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

std::string SqliteCursor::getString(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Dynamic::Var value = m_pRecordSet->value(columnIndex);
        std::string result;
        value.convert(result);
        return result;
    } catch (const Poco::InvalidAccessException& e) {
        EASYHTTPCPP_LOG_D(Tag, "getString Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getString Failed", e);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getString Failed columnIndex [%zu]", columnIndex);
        throw SqlExecutionException("getString Failed", e);
    }
}

std::string SqliteCursor::optGetString(size_t columnIndex, const std::string& defaultValue)
{
    try {
        return getString(columnIndex);
    } catch (...) {
        EASYHTTPCPP_LOG_D(Tag, "caught exception use default value");
    }
    return defaultValue;
}

CursorFieldType SqliteCursor::getType(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        Poco::Data::MetaColumn::ColumnDataType type = m_pRecordSet->columnType(columnIndex);

        CursorFieldType ret = CursorFieldTypeString;
        switch (type) {
            case Poco::Data::MetaColumn::FDT_INT8:
            case Poco::Data::MetaColumn::FDT_INT16:
            case Poco::Data::MetaColumn::FDT_INT32:
            case Poco::Data::MetaColumn::FDT_INT64:
                ret = CursorFieldTypeInteger;
                break;
            case Poco::Data::MetaColumn::FDT_FLOAT:
                ret = CursorFieldTypeFloat;
                break;
            case Poco::Data::MetaColumn::FDT_BLOB:
                ret = CursorFieldTypeBlob;
                break;
            default:
                ret = CursorFieldTypeString;
        }
        return ret;
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "getType Failed columnIndex [%d]", columnIndex);
        throw SqlIllegalArgumentException(Poco::format("invalid index %z", columnIndex), e);
    }
}

bool SqliteCursor::isNull(size_t columnIndex)
{
    throwExceptionIfIllegalState(__func__);

    try {
        std::vector<std::string> columnNames = SqliteCursor::getColumnNames();
        std::string columnName = columnNames.at(columnIndex);
        return m_pRecordSet->isNull(columnName);
    } catch (const Poco::Exception& e) {
        EASYHTTPCPP_LOG_D(Tag, "isNull Failed columnIndex [%d]", columnIndex);
        throw SqlIllegalArgumentException(Poco::format("invalid index %z", columnIndex), e);
    }
}

bool SqliteCursor::move(int offset)
{
    int position = static_cast<int> (m_currentRow) + offset;
    if (position < 0) {
        EASYHTTPCPP_LOG_D(Tag, "move failed for invalid offset current row[%d] offset[%d]", m_currentRow, offset);
        return false;
    }
    return moveToPosition(static_cast<unsigned int>(position));
}

bool SqliteCursor::moveToFirst()
{
    throwExceptionIfIllegalState(__func__);
    bool result = m_pRecordSet->moveFirst();
    if (result) {
        m_currentRow = 0;
    }
    return result;
}

bool SqliteCursor::moveToLast()
{
    throwExceptionIfIllegalState(__func__);
    bool result = m_pRecordSet->moveLast();
    if (result) {
        m_currentRow = m_pRecordSet->getTotalRowCount() - 1;
    }
    return result;
}

bool SqliteCursor::moveToNext()
{
    throwExceptionIfIllegalState(__func__);
    bool result = m_pRecordSet->moveNext();
    if (result) {
        m_currentRow++;
    }
    return result;
}

bool SqliteCursor::moveToPrevious()
{
    throwExceptionIfIllegalState(__func__);
    bool result = m_pRecordSet->movePrevious();
    if (result) {
        m_currentRow--;
    }
    return result;
}

bool SqliteCursor::moveToPosition(unsigned int position)
{
    throwExceptionIfIllegalState(__func__);

    size_t totalRowCount = m_pRecordSet->getTotalRowCount();
    if (position >= totalRowCount) {
        return false;
    }

    if (m_currentRow == static_cast<size_t> (position)) {
        return true;
    }

    bool result = moveToFirst();
    if (!result) return result;

    for (size_t i = 0; i < position; i++) {
        result = moveToNext();
        if (!result) break;
    }
    return result;
}

void SqliteCursor::throwExceptionIfIllegalState(const std::string& func)
{
    if (!m_pRecordSet) {
        EASYHTTPCPP_LOG_D(Tag, "[%s] Sqlite Cursor is already closed", func.c_str());
        throw SqlIllegalStateException("Sqlite Cursor is already closed ");
    }
}

} /* namespace db */
} /* namespace easyhttpcpp */
