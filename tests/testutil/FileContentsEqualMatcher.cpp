/*
 * Copyright 2017 Sony Corporation
 */

#include "Poco/Buffer.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"

#include "easyhttpcpp/common/StringUtil.h"
#include "FileContentsEqualMatcher.h"

using easyhttpcpp::common::StringUtil;

namespace easyhttpcpp {
namespace testutil {

FileContentsEqualMatcher::FileContentsEqualMatcher(const char* pExpectedData, size_t expectedDataBytes) :
        m_pExpectedData(pExpectedData), m_expectedDataBytes(expectedDataBytes)
{
}

FileContentsEqualMatcher::~FileContentsEqualMatcher()
{
}

bool FileContentsEqualMatcher::MatchAndExplain(const std::string& filePath,
        testing::MatchResultListener* listener) const
{
    Poco::File targetFile(filePath);
    if (targetFile.getSize() != m_expectedDataBytes) {
        *listener << StringUtil::format("data bytes is different. file size is %zu bytes",
                targetFile.getSize());
        return false;
    }
    Poco::FileInputStream targetFileStream(targetFile.path(), std::ios::in | std::ios::binary);
    Poco::Buffer<char> inBuffer(m_expectedDataBytes);
    targetFileStream.read(inBuffer.begin(), m_expectedDataBytes);
    if (static_cast<size_t>(targetFileStream.gcount()) != m_expectedDataBytes) {
        *listener << StringUtil::format("the size of the read file is different. (%zd bytes)",
                targetFileStream.gcount());
        return false;
    }
    if (memcmp(inBuffer.begin(), m_pExpectedData, m_expectedDataBytes) != 0) {
        *listener << StringUtil::format("data is different. %s",
                exchangeDataToDisplayString(inBuffer.begin(), m_expectedDataBytes).c_str());
        return false;
    }
    return true;
}

void FileContentsEqualMatcher::DescribeTo(std::ostream* os) const
{
    *os << StringUtil::format("is equal to %s",
            exchangeDataToDisplayString(m_pExpectedData, m_expectedDataBytes).c_str());
}

void FileContentsEqualMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << StringUtil::format("is not equal to %s",
            exchangeDataToDisplayString(m_pExpectedData, m_expectedDataBytes).c_str());
}

std::string FileContentsEqualMatcher::exchangeDataToDisplayString(const char* pData, size_t dataBytes) const
{
    for (size_t i = 0; i < dataBytes; i++) {
        if ((pData[i] < 0x20 || pData[i] > 0x7e) && pData[i] != '\r' && pData[i] != '\n' && pData[i] == '\t') {
            return StringUtil::format("binary data (%zu bytes)", dataBytes);
        }
    }
    Poco::Buffer<char> buffer(dataBytes + 1);
    memcpy(buffer.begin(), pData, dataBytes);
    *(buffer.begin() + dataBytes) = '\0';
    return StringUtil::format("\"%s\" (%zu bytes)", buffer.begin(), dataBytes);
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
