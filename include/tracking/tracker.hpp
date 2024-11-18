#pragma once

#include <tracking/base.hpp>
#include <tracking/sort.hpp>
#include <tracking/botsort.hpp>

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

inline TrackerType getTrackerType(std::string name)
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

class TrackerFactory
{
public:
    static std::unique_ptr<BaseTracker> create(TrackerType tracker, const std::string &config_file)
    {
        switch (tracker)
        {
        case (TrackerType::SORT):
        {
            auto config = isFileAccessible(config_file) ? SortConfig::load(config_file) : SortConfig();
            return std::make_unique<Sort>(config);
        }
        case (TrackerType::BOTSORT):
        {
            auto config = isFileAccessible(config_file) ? BotSortConfig::load(config_file) : BotSortConfig();
            return std::make_unique<BotSort>(config);
        }
        default:
            throw std::runtime_error("Unknown tracker type");
        }
    }
};