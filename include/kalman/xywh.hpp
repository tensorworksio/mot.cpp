#pragma once

#include "kalman.hpp"

class KalmanFilterXYWH : public BaseKalmanFilter
{
public:
    KalmanFilterXYWH(const cv::Rect2f &rect) : BaseKalmanFilter() { init(rect); };
    KalmanFilterXYWH(const cv::Rect2f &rect, const KalmanConfig &config) : BaseKalmanFilter(config) { init(rect); };

    void update(const cv::Rect2f &rect) override;
    void reset() override;

    cv::Rect2f getBox(const cv::Mat &state) override;
    cv::Point2f getVelocity(const cv::Mat &state) override;

private:
    void init(const cv::Rect2f &rect) override;
};
