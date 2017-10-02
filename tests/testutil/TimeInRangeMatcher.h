/*
 * Copyright 2017 Sony Corporation
 */

#ifndef EASYHTTPCPP_TESTUTIL_TIMEINRANGEMATCHER_H_INCLUDED
#define EASYHTTPCPP_TESTUTIL_TIMEINRANGEMATCHER_H_INCLUDED

#include <ctime>
#include "gmock/gmock.h"

#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timestamp.h"

#include "easyhttpcpp/common/StringUtil.h"

namespace easyhttpcpp {
namespace testutil {

template<typename T>
class TimeInRangeMatcher {
public:
    TimeInRangeMatcher(T startAtEpoch, T endAtEpoch) : m_startAtEpoch(startAtEpoch), m_endAtEpoch(endAtEpoch)
    {
    }

    TimeInRangeMatcher(const TimeInRangeMatcher& matcher) : m_startAtEpoch(matcher.m_startAtEpoch),
            m_endAtEpoch(matcher.m_endAtEpoch)
    {
    }
    virtual ~TimeInRangeMatcher()
    {
    }

    bool MatchAndExplain(T actual, testing::MatchResultListener* listener) const
    {
        if (!(actual >= m_startAtEpoch && actual <= m_endAtEpoch)) {
            Poco::Timestamp actualTime = Poco::Timestamp::fromEpochTime(actual);
            *listener <<  "(" << Poco::DateTimeFormatter::format(actualTime, Poco::DateTimeFormat::HTTP_FORMAT) << ")";
            return false;
        }
        return true;
    }

    void DescribeTo(std::ostream* os) const
    {
        Poco::Timestamp startTime = Poco::Timestamp::fromEpochTime(m_startAtEpoch);
        Poco::Timestamp endTime = Poco::Timestamp::fromEpochTime(m_endAtEpoch);

        *os << "is in range " <<
                easyhttpcpp::common::StringUtil::format("%lld (", static_cast<long long>(m_startAtEpoch)) <<
                Poco::DateTimeFormatter::format(startTime, Poco::DateTimeFormat::HTTP_FORMAT) <<
                ") and " <<
                easyhttpcpp::common::StringUtil::format("%lld (", static_cast<long long>(m_endAtEpoch)) <<
                Poco::DateTimeFormatter::format(endTime, Poco::DateTimeFormat::HTTP_FORMAT) << ")";
    }
    void DescribeNegationTo(std::ostream* os) const
    {
        Poco::Timestamp startTime = Poco::Timestamp::fromEpochTime(m_startAtEpoch);
        Poco::Timestamp endTime = Poco::Timestamp::fromEpochTime(m_endAtEpoch);

        *os << "is not in range " <<
                easyhttpcpp::common::StringUtil::format("%lld (", static_cast<long long>(m_startAtEpoch)) <<
                Poco::DateTimeFormatter::format(startTime, Poco::DateTimeFormat::HTTP_FORMAT) <<
                ") and "
                << easyhttpcpp::common::StringUtil::format("%lld (", static_cast<long long>(m_endAtEpoch)) <<
                Poco::DateTimeFormatter::format(endTime, Poco::DateTimeFormat::HTTP_FORMAT) << ")";
    }

private:
    T m_startAtEpoch;
    T m_endAtEpoch;
};

template<typename T>
inline testing::PolymorphicMatcher<TimeInRangeMatcher<T> > isTimeInRange(T startAtEpoch, T endAtEpoch)
{
	return testing::MakePolymorphicMatcher(TimeInRangeMatcher<T>(startAtEpoch, endAtEpoch));
}

} /* namespace testutil */
} /* namespace easyhttpcpp */

#endif /* EASYHTTPCPP_TESTUTIL_TIMEINRANGEMATCHER_H_INCLUDED */
