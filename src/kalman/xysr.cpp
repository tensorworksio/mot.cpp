#include <kalman/xysr.hpp>

void KalmanFilterXYSR::init(const cv::Rect2f &rect)
{
    size_t stateNum = 7;   // [xc, yc, s, r, dxc, dyc, ds]
    size_t measureNum = 4; // [x, y, s, r]

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
    for (size_t i = 0; i < stateNum - measureNum; ++i)
    {
        kf.processNoiseCov.at<float>(i, measureNum + i) = 1;
        kf.processNoiseCov.at<float>(measureNum + i, i) = 1;
    }

    kf.processNoiseCov *= process_noise_scale;
    kf.processNoiseCov.at<float>(stateNum - 1, stateNum - 1) *= 0.01f;
    kf.processNoiseCov.rowRange(measureNum, stateNum).colRange(measureNum, stateNum) *= 0.01f;

    // Measurement covariance matrix
    kf.measurementNoiseCov = cv::Mat::eye(measureNum, measureNum, CV_32F);
    kf.measurementNoiseCov *= measurement_noise_scale;
    kf.measurementNoiseCov.rowRange(measureNum / 2, measureNum).colRange(measureNum / 2, measureNum) *= 0.01f;

    // Error covariance matrix
    kf.errorCovPost = cv::Mat::eye(stateNum, stateNum, CV_32F);
    kf.errorCovPost *= 10.f;
    kf.errorCovPost.rowRange(measureNum, stateNum).colRange(measureNum, stateNum) *= 100.f;

    // Initial state
    kf.statePost = cv::Mat::zeros(stateNum, 1, CV_32F);
    kf.statePost.at<float>(0, 0) = rect.x + rect.width / 2.f;
    kf.statePost.at<float>(1, 0) = rect.y + rect.height / 2.f;
    kf.statePost.at<float>(2, 0) = rect.area();
    kf.statePost.at<float>(3, 0) = rect.area() ? rect.width / rect.height : 0.f;
}

void KalmanFilterXYSR::update(const cv::Rect2f &rect)
{
    measurement.at<float>(0, 0) = rect.x + rect.width / 2.f;
    measurement.at<float>(1, 0) = rect.y + rect.height / 2.f;
    measurement.at<float>(2, 0) = rect.area();
    measurement.at<float>(3, 0) = rect.area() ? rect.width / rect.height : 0.f;
    kf.correct(measurement);
}

void KalmanFilterXYSR::reset()
{
    kf.statePost.at<float>(6, 0) = 0.f;
}

cv::Rect2f KalmanFilterXYSR::getBox(const cv::Mat &state)
{
    float area = std::max(0.f, state.at<float>(2));
    float width = std::sqrt(std::max(0.f, area * state.at<float>(3)));
    float height = width ? area / width : 0.f;
    float x_left = std::max(0.f, state.at<float>(0) - width / 2.f);
    float y_top = std::max(0.f, state.at<float>(1) - height / 2.f);
    return cv::Rect2f(x_left, y_top, width, height);
}

cv::Point2f KalmanFilterXYSR::getVelocity(const cv::Mat &state)
{
    float dx = state.at<float>(4);
    float dy = state.at<float>(5);
    return cv::Point2f(dx, dy);
}
