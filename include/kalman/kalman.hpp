#pragma once

#include <opencv2/opencv.hpp>

struct KalmanConfig
{
    int time_step = 1;
    float process_noise_scale = 1.f;
    float measurement_noise_scale = 1.f;
};

class BaseKalmanFilter
{
public:
    BaseKalmanFilter(int dt = 1, float t_process_noise_scale = 1.f, float t_meas_noise_scale = 1.f)
        : time_step(static_cast<float>(dt)), process_noise_scale(t_process_noise_scale), measurement_noise_scale(t_meas_noise_scale) {};

    BaseKalmanFilter(const KalmanConfig &config) : BaseKalmanFilter(config.time_step, config.process_noise_scale, config.measurement_noise_scale) {};

    virtual ~BaseKalmanFilter() {};
    virtual void reset() = 0;
    virtual void update(const cv::Rect2f &rect) = 0;

    virtual cv::Rect2f getBox(const cv::Mat &state) = 0;
    cv::Rect2f getBox() { return getBox(kf.statePost); };

    virtual cv::Point2f getVelocity(const cv::Mat &state) = 0;
    cv::Point2f getVelocity() { return getVelocity(kf.statePost); };

    cv::Rect2f predict() { return getBox(kf.predict()); };

protected:
    float time_step;
    float process_noise_scale;
    float measurement_noise_scale;

    cv::KalmanFilter kf{};
    cv::Mat measurement{};

private:
    virtual void init(const cv::Rect2f &rect) = 0;
};