#pragma once

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

struct BotSortConfig
{
    using Tag = rfl::Literal<"botsort">;

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