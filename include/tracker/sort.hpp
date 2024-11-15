#pragma once

#include <fstream>
#include <tracker/base.hpp>
#include <utils/json_utils.hpp>

struct SortConfig : public JsonConfig
{
    KalmanConfig kalman{};
    size_t max_time_lost = 15;
    float match_thresh = 0.3f;

    std::shared_ptr<const JsonConfig> clone() const override { return std::make_shared<SortConfig>(*this); }

    void loadFromJson(const nlohmann::json &data) override
    {
        if (data.contains("kalman"))
            kalman.loadFromJson(data["kalman"]);
        if (data.contains("max_time_lost"))
            max_time_lost = data["max_time_lost"].get<size_t>();
        if (data.contains("match_thresh"))
            match_thresh = data["match_thresh"].get<float>();
    }

    static SortConfig load(const std::string &filename)
    {
        std::ifstream file(filename);
        auto data = nlohmann::json::parse(file);

        SortConfig config;
        config.loadFromJson(data["sort"]);
        return config;
    }
};

class Sort : public BaseTracker
{
public:
    Sort(const SortConfig &t_config) : config(t_config) {}
    const SortConfig &getConfig() const { return config; };
    void process(Frame &frame) override;

private:
    const SortConfig config;
    void assign(std::vector<Detection> &detections,
                float match_thresh,
                std::set<std::pair<size_t, size_t>> &matches,
                std::set<size_t> &unmatched_detections,
                std::set<size_t> &unmatched_tracks);
};