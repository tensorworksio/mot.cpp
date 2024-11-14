#include <kalman/xywh.hpp>

void KalmanFilterXYWH::init(const cv::Rect2f &rect)
{
    size_t stateNum = 8;   // [xc, yc, w, h, dxc, dyc, dw, dh]
    size_t measureNum = 4; // [xc, yc, w, h]

    float std_weight_position = 5e-2;
    float std_weight_velocity = 625e-5;

    kf = cv::KalmanFilter(stateNum, measureNum, 0);
    measurement = cv::Mat::zeros(measureNum, 1, CV_32F);

    // Transition matrix
    kf.transitionMatrix = cv::Mat::eye(stateNum, stateNum, CV_32F);
    for (size_t i = 0; i < stateNum - measureNum; ++i)
    {
        kf.transitionMatrix.at<float>(i, measureNum + i) = time_step;
    }

    // Measurement matrix
    kf.measurementMatrix = cv::Mat::eye(measureNum, stateNum, CV_32F);

    // Process covariance matrix
    kf.processNoiseCov = cv::Mat::eye(stateNum, stateNum, CV_32F);
    kf.processNoiseCov *= process_noise_scale;
    for (size_t i = 0; i < measureNum; ++i)
    {
        kf.processNoiseCov.at<float>(i, i) *= std_weight_position;
    }
    for (size_t i = measureNum; i < stateNum; ++i)
    {
        kf.processNoiseCov.at<float>(i, i) *= std_weight_velocity;
    }

    // Measurement covariance matrix
    kf.measurementNoiseCov = cv::Mat::eye(measureNum, measureNum, CV_32F);
    kf.measurementNoiseCov *= measurement_noise_scale;
    for (size_t i = 0; i < measureNum; ++i)
    {
        kf.measurementNoiseCov.at<float>(i, i) *= std_weight_position;
    }

    // Error covariance matrix
    kf.errorCovPost = cv::Mat::eye(stateNum, stateNum, CV_32F);
    for (size_t i = 0; i < measureNum; ++i)
    {
        kf.errorCovPost.at<float>(i, i) = std::pow(2 * std_weight_position * (i % 2 ? rect.height : rect.width), 2);
    }
    for (size_t i = measureNum; i < stateNum; ++i)
    {
        kf.errorCovPost.at<float>(i, i) = std::pow(10 * std_weight_velocity * (i % 2 ? rect.height : rect.width), 2);
    }

    // Initial state
    kf.statePost = cv::Mat::zeros(stateNum, 1, CV_32F);
    kf.statePost.at<float>(0, 0) = rect.x + rect.width / 2.f;
    kf.statePost.at<float>(1, 0) = rect.y + rect.height / 2.f;
    kf.statePost.at<float>(2, 0) = rect.width;
    kf.statePost.at<float>(3, 0) = rect.height;
}

void KalmanFilterXYWH::update(const cv::Rect2f &rect)
{
    measurement.at<float>(0, 0) = rect.x + rect.width / 2.f;
    measurement.at<float>(1, 0) = rect.y + rect.height / 2.f;
    measurement.at<float>(2, 0) = rect.width;
    measurement.at<float>(3, 0) = rect.height;
    kf.correct(measurement);
}

void KalmanFilterXYWH::reset()
{
    kf.statePost.at<float>(6, 0) = 0.f;
    kf.statePost.at<float>(7, 0) = 0.f;
}

cv::Rect2f KalmanFilterXYWH::getBox(const cv::Mat &state)
{
    float width = std::max(0.f, state.at<float>(2));
    float height = std::max(0.f, state.at<float>(3));
    float x = std::max(0.f, state.at<float>(0) - width / 2.f);
    float y = std::max(0.f, state.at<float>(1) - height / 2.f);
    return cv::Rect2f(x, y, width, height);
}

cv::Point2f KalmanFilterXYWH::getVelocity(const cv::Mat &state)
{
    float dx = state.at<float>(4);
    float dy = state.at<float>(5);
    return cv::Point2f(dx, dy);
}