/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_COMMON_STRINGUTIL_H_INCLUDED
#define EASYHTTPCPP_COMMON_STRINGUTIL_H_INCLUDED

#include <string>

namespace easyhttpcpp {
namespace common {

class StringUtil {
public:
    /**
     * @brief format is string formatter.
     *      %c  char
     *      %s  char *
     *      %d  int, short
     *      %u  unsigned int, unsigned short
     *      %o  int, short, unsigned int, unsigned short
     *      %x  int, short, unsigned int, unsigned short
     *      %f  float
     *      %e  float
     *      %g  float
     *      %ld long
     *      %lu unsigned long
     *      %lo long, unsigned long
     *      %lx long, unsigned long
     *      %lf double
     *      %lld long long
     *      %llu unsigned long long
     * 
     * @param pFormat
     * @param ...
     * @return the formated string.
     */
    static std::string format(const char* pFormat, ...);
    static const char* boolToString(bool boolean);
    static bool endsWith(const std::string& str, const std::string& end);
    static bool startsWith(const std::string& str, const std::string& start);

    static bool isNullOrEmpty(const std::string* str);

    static std::string formatVersion(const std::string& major, const std::string& minor, const std::string& patch,
            const std::string& extension);

private:
    StringUtil();
    virtual ~StringUtil();
};

} /* namespace common */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_COMMON_STRINGUTIL_H_INCLUDED */
