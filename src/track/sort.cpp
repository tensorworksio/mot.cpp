#include <track/sort.hpp>
#include <kalman/xywh.hpp>

SortTrack::SortTrack(const cv::Rect2f &rect, const KalmanConfig &config) : BaseTrack(std::make_shared<KalmanFilterXYWH>(rect, config)) {}

void SortTrack::predict()
{
    if (!isActive())
        kf->reset();

    BaseTrack::predict();
}

void SortTrack::update(Detection &det)
{
    BaseTrack::update(det);
}