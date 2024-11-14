#pragma once

#include <opencv2/opencv.hpp>
#include <utils/json_utils.hpp>

struct KalmanConfig : public JsonConfig
{
    int time_step = 1;
    float process_noise_scale = 1.f;
    float measurement_noise_scale = 1.f;

    std::shared_ptr<const JsonConfig> clone() const override { return std::make_shared<KalmanConfig>(*this); }

    void loadFromJson(const nlohmann::json &data) override
    {
        if (data.contains("time_step"))
            time_step = data["time_step"].get<int>();
        if (data.contains("process_noise_scale"))
            process_noise_scale = data["process_noise_scale"].get<float>();
        if (data.contains("measurement_noise_scale"))
            measurement_noise_scale = data["measurement_noise_scale"].get<float>();
    }
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