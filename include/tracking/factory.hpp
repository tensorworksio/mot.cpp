#pragma once

#include <fstream>
#include <rfl/json.hpp>

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
        struct TrackerFile {
            rfl::TaggedUnion<"name", SortConfig, BotSortConfig> tracker;
        };

        std::ifstream file(config_file);
        auto result = rfl::json::read<TrackerFile, rfl::DefaultIfMissing>(file);
        if (!result.has_value())
            throw std::runtime_error(result.error().what());

        return result.value().tracker.visit([](auto &&config) -> std::unique_ptr<BaseTracker> {
            using T = std::decay_t<decltype(config)>;
            if constexpr (std::is_same_v<T, SortConfig>)
                return std::make_unique<Sort>(config);
            else if constexpr (std::is_same_v<T, BotSortConfig>)
                return std::make_unique<BotSort>(config);
        });
    }
};
