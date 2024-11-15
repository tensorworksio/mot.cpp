#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <array>
#include <common/frame.hpp>
#include <track/base.hpp>

constexpr float PRECISION = 1E6f;

enum class TrackerType
{
    SORT,
    BOTSORT,
    UNKNOWN
};

inline std::string getTrackerName(TrackerType type)
{
    switch (type)
    {
    case TrackerType::SORT:
        return "sort";
    case TrackerType::BOTSORT:
        return "botsort";
    default:
        throw std::runtime_error("Unknown tracker type");
    }
}

inline auto &getTrackers()
{
    static std::array<TrackerType, 2> trackers{TrackerType::SORT, TrackerType::BOTSORT};
    return trackers;
}

inline TrackerType getTrackerType(std::string &name)
{
    for (const auto &tracker_type : getTrackers())
    {
        if (name == getTrackerName(tracker_type))
        {
            return tracker_type;
        }
    }
    return TrackerType::UNKNOWN;
}

class BaseTracker
{
public:
    BaseTracker() = default;
    virtual ~BaseTracker() = default;
    virtual void process(Frame &frame) = 0;

protected:
    std::vector<std::unique_ptr<BaseTrack>> tracks{};
};