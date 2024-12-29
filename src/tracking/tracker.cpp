#include <tracking/tracker.hpp>

size_t BaseTrack::count = 0;

BaseTrack::BaseTrack(std::shared_ptr<BaseKalmanFilter> kalman_filter)
    : kf(kalman_filter)
{
    id = getNextId();
    history.reserve(MAX_HISTORY);
}

BaseTrack::~BaseTrack()
{
    history.clear();
}

void BaseTrack::update(Detection &det)
{
    time_since_update = 0;
    history.clear();
    markActive();
    kf->update(det.bbox);
}

void BaseTrack::predict()
{
    age++;
    time_since_update++;
    auto pred = kf->predict();
    if (history.size() >= MAX_HISTORY)
        history.erase(history.begin());
    history.push_back(pred);
}

cv::Rect2f BaseTrack::getBox() const
{
    return kf->getBox();
}

cv::Point2f BaseTrack::getVelocity() const
{
    return kf->getVelocity();
}