#pragma once

#include <track/base.hpp>

struct SortTrack : BaseTrack
{
    SortTrack(const cv::Rect2f &rect, const KalmanConfig &config);
    void predict() override;
    void update(Detection &det) override;
};