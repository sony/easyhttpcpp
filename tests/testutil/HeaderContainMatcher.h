/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_HEADERCONTAINMATCHER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_HEADERCONTAINMATCHER_H_INCLUDED

#include <string>
#include "gmock/gmock.h"

#include "easyhttpcpp/Headers.h"

namespace easyhttpcpp {
namespace testutil {

class HeaderContainMatcher {
public:
    HeaderContainMatcher(const std::string& name, const std::string& value);
    HeaderContainMatcher(const std::string& name);
    virtual ~HeaderContainMatcher();

    bool MatchAndExplain(easyhttpcpp::Headers::Ptr pActual, testing::MatchResultListener* listener) const;

    void DescribeTo(std::ostream* os) const;
    void DescribeNegationTo(std::ostream* os) const;

private:
    std::string m_name;
    std::string m_value;
    bool m_keyOnly;
};

inline testing::PolymorphicMatcher<HeaderContainMatcher> containsInHeader(const std::string& name,
        const std::string& value)
{
	return testing::MakePolymorphicMatcher(HeaderContainMatcher(name, value));
}

inline testing::PolymorphicMatcher<HeaderContainMatcher> hasKeyInHeader(const std::string& name)
{
	return testing::MakePolymorphicMatcher(HeaderContainMatcher(name));
}

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_HEADERCONTAINMATCHER_H_INCLUDED */
