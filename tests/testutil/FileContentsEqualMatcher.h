/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_FILECONTENTSEQUALMATCHER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_FILECONTENTSEQUALMATCHER_H_INCLUDED

#include <string>
#include "gmock/gmock.h"

#include "Poco/File.h"

namespace easyhttpcpp {
namespace testutil {

class FileContentsEqualMatcher {
public:
    FileContentsEqualMatcher(const char* pExpectedData, size_t expectedDataBytes);
    virtual ~FileContentsEqualMatcher();

    bool MatchAndExplain(const std::string& filePath, testing::MatchResultListener* listener) const;

    void DescribeTo(std::ostream* os) const;
    void DescribeNegationTo(std::ostream* os) const;

private:
    std::string exchangeDataToDisplayString(const char* pData, size_t dataBytes) const;

    const char* m_pExpectedData;
    const size_t m_expectedDataBytes;
};

inline testing::PolymorphicMatcher<FileContentsEqualMatcher> equalsContentsOfFile(const char* pExpectedData,
        size_t expectedDataBytes)
{
	return testing::MakePolymorphicMatcher(FileContentsEqualMatcher(pExpectedData, expectedDataBytes));
}

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_FILECONTENTSEQUALMATCHER_H_INCLUDED */
