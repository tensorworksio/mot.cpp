#pragma once

#include "tracker.hpp"

struct SortTrack : BaseTrack
{
    SortTrack(const cv::Rect2f &rect, const KalmanConfig &config);
    void predict() override;
    void update(Detection &det) override;
};

struct SortConfig
{
    using Tag = rfl::Literal<"sort">;

    KalmanConfig kalman{};

    size_t max_time_lost = 15;
    float match_thresh = 0.3f;
};

class Sort : public BaseTracker
{
public:
    Sort(const SortConfig &t_config) : config(t_config) {}
    const SortConfig &getConfig() const { return config; };
    void update(std::vector<Detection> &detections) override;

private:
    const SortConfig config;
    void assign(std::vector<Detection> &detections,
                float match_thresh,
                std::set<std::pair<size_t, size_t>> &matches,
                std::set<size_t> &unmatched_detections,
                std::set<size_t> &unmatched_tracks);
};