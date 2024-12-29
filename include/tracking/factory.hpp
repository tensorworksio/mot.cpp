#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

#include "tracker.hpp"
#include "sort.hpp"
#include "botsort.hpp"

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

inline TrackerType getTrackerType(const std::string &name)
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
    static std::unique_ptr<BaseTracker> create(const std::string &config_file)
    {
        std::ifstream file(config_file);
        auto data = nlohmann::json::parse(file);
        TrackerType tracker_type = getTrackerType(data["tracker"]["name"]);

        switch (tracker_type)
        {
        case TrackerType::SORT:
        {
            auto config = SortConfig();
            config.loadFromJson(data["tracker"]);
            return std::make_unique<Sort>(config);
        }
        case TrackerType::BOTSORT:
        {
            auto config = BotSortConfig();
            config.loadFromJson(data["tracker"]);
            return std::make_unique<BotSort>(config);
        }
        default:
            throw std::runtime_error("Unknown tracker type");
        }
    }
};