/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_STDCLOGCAPTURE_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_STDCLOGCAPTURE_H_INCLUDED

#include <streambuf>

#include "Poco/Buffer.h"
#include "Poco/MemoryStream.h"
#include "Poco/SharedPtr.h"

#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

class EASYHTTPCPP_TESTUTIL_API StdCLogCapture {
public:
    typedef std::vector<std::string> CapturedLinesVec;

    StdCLogCapture(size_t captureSizeMax);
    virtual ~StdCLogCapture();

    void startCapture();
    CapturedLinesVec getCapture();
    void endCapture();

private:
    std::streambuf* m_pOldRdbuf;
    Poco::SharedPtr< Poco::Buffer<char> > m_pPocoBuffer;
    Poco::SharedPtr< Poco::MemoryOutputStream > m_pPocoMemoryStream;
};

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_STDCLOGCAPTURE_H_INCLUDED */
