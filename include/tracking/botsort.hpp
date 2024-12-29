#pragma once

#include <fstream>
#include <utils/json_utils.hpp>

#include "tracker.hpp"

struct BotSortTrack : BaseTrack
{
    float alpha = 0.9f;
    std::vector<float> features{};

    BotSortTrack(const cv::Rect2f &rect, const KalmanConfig &config);
    BotSortTrack(const cv::Rect2f &rect, const std::vector<float> &feat, const KalmanConfig &config);
    void predict() override;
    void update(Detection &det) override;
};

struct BotSortConfig : public JsonConfig
{
    KalmanConfig kalman{};

    size_t max_time_lost = 15;

    float track_high_thresh = 0.5f;
    float track_low_thresh = 0.1f;
    float new_track_thresh = 0.6f;

    float first_match_thresh = 0.3f;
    float second_match_thresh = 0.1f;
    float unconfirmed_match_thresh = 0.2f;

    float proximity_thresh = 0.5f;
    float appearance_thresh = 0.9f;

    std::shared_ptr<const JsonConfig> clone() const override { return std::make_shared<BotSortConfig>(*this); }

    void loadFromJson(const nlohmann::json &data) override
    {
        if (data.contains("kalman"))
            kalman.loadFromJson(data["kalman"]);
        if (data.contains("max_time_lost"))
            max_time_lost = data["max_time_lost"].get<size_t>();
        if (data.contains("track_high_thresh"))
            track_high_thresh = data["track_high_thresh"].get<float>();
        if (data.contains("track_low_thresh"))
            track_low_thresh = data["track_low_thresh"].get<float>();
        if (data.contains("new_track_thresh"))
            new_track_thresh = data["new_track_thresh"].get<float>();
        if (data.contains("first_match_thresh"))
            first_match_thresh = data["first_match_thresh"].get<float>();
        if (data.contains("second_match_thresh"))
            second_match_thresh = data["second_match_thresh"].get<float>();
        if (data.contains("unconfirmed_match_thresh"))
            unconfirmed_match_thresh = data["unconfirmed_match_thresh"].get<float>();
        if (data.contains("proximity_thresh"))
            proximity_thresh = data["proximity_thresh"].get<float>();
        if (data.contains("appearance_thresh"))
            appearance_thresh = data["appearance_thresh"].get<float>();
    }
};

class BotSort : public BaseTracker
{
public:
    BotSort(const BotSortConfig &t_config) : config(t_config) {}
    const BotSortConfig &getConfig() const { return config; };
    void update(std::vector<Detection> &detections) override;

private:
    const BotSortConfig config;
    void assign(std::vector<Detection *> &dets,
                std::vector<BotSortTrack *> &trks,
                float match_thresh,
                float proximity_thresh,
                float appearance_thresh,
                std::set<std::pair<size_t, size_t>> &matches,
                std::set<size_t> &unmatched_detections,
                std::set<size_t> &unmatched_tracks);
};