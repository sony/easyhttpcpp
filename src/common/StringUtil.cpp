/*
 * Copyright 2017 Sony Corporation
 */

#include <string>
#include <stdio.h>
#include <stdarg.h>

#include "easyhttpcpp/common/StringUtil.h"

#define VSCPRINTF(format, ...) vsnprintf(NULL, 0, format, __VA_ARGS__)
// Include the termination character to size.
#define VSNPRINTF(s, s_size, n, format, ...) vsnprintf(s, (n)+1, format, __VA_ARGS__)

namespace easyhttpcpp {
namespace common {

StringUtil::StringUtil()
{
}

StringUtil::~StringUtil()
{
}

std::string StringUtil::format(const char* pFormat, ...)
{
    if (!pFormat) {
        return "";
    }

    va_list argp;

    va_start(argp, pFormat);
    int len = VSCPRINTF(pFormat, argp);
    // TODO use Poco::Buffer here
    char* pResult = new char[static_cast<size_t>(len + 1)];
    va_end(argp);

    va_start(argp, pFormat);
    VSNPRINTF(pResult, len + 1, static_cast<size_t>(len), pFormat, argp);
    va_end(argp);

    std::string retString(pResult);
    delete []pResult;
    pResult = NULL;

    return retString;
}

// Macros - not type-safe, has side-effects. Use this function instead
//#define BOOL_STR(b) ((b)?"true":"false")

const char* StringUtil::boolToString(bool boolean)
{
    return boolean ? "true" : "false";
}

bool StringUtil::endsWith(const std::string& str, const std::string& end)
{
    if (str.size() < end.size()) {
        return false;
    }

    return (str.compare(str.size() - end.size(), end.size(), end) == 0);
}

bool StringUtil::startsWith(const std::string& str, const std::string& start)
{
    if (str.size() < start.size()) {
        return false;
    }

    return (str.compare(0, start.size(), start) == 0);
}

bool StringUtil::isNullOrEmpty(const std::string* str)
{
    return !str || str->empty();
}

std::string StringUtil::formatVersion(const std::string& major, const std::string& minor, const std::string& patch,
        const std::string& extension)
{
    if (extension.empty()) {
        return format("%s.%s.%s", major.c_str(), minor.c_str(), patch.c_str());
    } else {
        return format("%s.%s.%s-%s", major.c_str(), minor.c_str(), patch.c_str(), extension.c_str());
    }
}

} /* namespace common */
} /* namespace easyhttpcpp */
