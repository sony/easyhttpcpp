/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_HEADERSEQUALMATCHER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_HEADERSEQUALMATCHER_H_INCLUDED

#include <string>
#include "gmock/gmock.h"

#include "easyhttpcpp/Headers.h"
#include "TestUtilExports.h"

namespace easyhttpcpp {
namespace testutil {

class EASYHTTPCPP_TESTUTIL_API HeadersEqualMatcher {
public:
    HeadersEqualMatcher(easyhttpcpp::Headers::Ptr pHeaders);

    virtual ~HeadersEqualMatcher();

    bool MatchAndExplain(easyhttpcpp::Headers::Ptr pActual, testing::MatchResultListener* listener) const;

    void DescribeTo(std::ostream* os) const;
    void DescribeNegationTo(std::ostream* os) const;

private:
    easyhttpcpp::Headers::Ptr m_pExpectedHeaders;
};

inline testing::PolymorphicMatcher<HeadersEqualMatcher> equalHeaders(easyhttpcpp::Headers::Ptr pHeaders)
{
	return testing::MakePolymorphicMatcher(HeadersEqualMatcher(pHeaders));
}

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_HEADERSEQUALMATCHER_H_INCLUDED */
