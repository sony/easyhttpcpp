/*
 * Copyright 2017 Sony Corporation
 */

#include <iostream>

#include "Poco/Util/SystemConfiguration.h"

#include "StdCLogCapture.h"

namespace easyhttpcpp {
namespace testutil {

static const unsigned int StdCLogCaptureReadLineSizeMax = 4096;
static const char* const StdCLogCaptureAnchorStart =
        "############################# \n clog capture start.\n#############################";
static const char* const StdCLogCaptureAnchorEnd =
        "############################# \n clog capture end.\n#############################";

StdCLogCapture::StdCLogCapture(size_t captureSizeMax) : m_pOldRdbuf(NULL),
        m_pPocoBuffer(new Poco::Buffer<char>(captureSizeMax)),
        m_pPocoMemoryStream(new Poco::MemoryOutputStream(m_pPocoBuffer->begin(), m_pPocoBuffer->size()))
{
    m_pPocoBuffer->clear();
}

StdCLogCapture::~StdCLogCapture()
{
    endCapture();
}

void StdCLogCapture::startCapture()
{
    // redirect std::clog to MemoryInputStream.
    // save old buffer.
    std::cout << StdCLogCaptureAnchorStart << std::endl;
    m_pOldRdbuf = std::clog.rdbuf(m_pPocoMemoryStream->rdbuf());
}

StdCLogCapture::CapturedLinesVec StdCLogCapture::getCapture()
{
    std::vector<std::string> result;

    Poco::MemoryInputStream istr(m_pPocoBuffer->begin(), strlen(m_pPocoBuffer->begin()));
    Poco::Buffer<char> buffer(StdCLogCaptureReadLineSizeMax);

    // read stream to string
    istr.getline(buffer.begin(), buffer.size());
    while (strlen(buffer.begin()) != 0) {
        result.push_back(buffer.begin());
        buffer.clear();
        istr.getline(buffer.begin(), buffer.size());
    }

    return result;
}

void StdCLogCapture::endCapture()
{
    //reset clog
    if (m_pOldRdbuf) {
        std::clog.rdbuf(m_pOldRdbuf);
        m_pOldRdbuf = NULL;
        std::cout << StdCLogCaptureAnchorEnd << std::endl;
    }
}

} /* namespace testutil */
} /* namespace easyhttpcpp */
