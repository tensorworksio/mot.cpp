#pragma once

#include "base.hpp"

class KalmanFilterXYSR : public BaseKalmanFilter
{
public:
    KalmanFilterXYSR(const cv::Rect2f &rect) : BaseKalmanFilter() { init(rect); };
    KalmanFilterXYSR(const cv::Rect2f &rect, const KalmanConfig &config) : BaseKalmanFilter(config) { init(rect); };

    void reset() override;
    void update(const cv::Rect2f &rect) override;
    cv::Rect2f getBox(const cv::Mat &state) override;
    cv::Point2f getVelocity(const cv::Mat &state) override;

private:
    void init(const cv::Rect2f &rect) override;
};
