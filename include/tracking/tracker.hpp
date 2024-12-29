#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <array>

#include <types/frame.hpp>
#include <types/detection.hpp>

#include <kalman/kalman.hpp>

constexpr float PRECISION = 1E6f;
constexpr size_t MAX_HISTORY = 50;

enum class TrackState : int
{
    New = 0,
    Tracked = 1,
    Lost = 2,
    Removed = 3
};

struct BaseTrack
{
    static size_t count;
    int id = 0;
    size_t age = 0;
    size_t time_since_update = 0;
    TrackState state = TrackState::New;
    std::vector<cv::Rect2f> history;
    std::shared_ptr<BaseKalmanFilter> kf = nullptr;

    BaseTrack(std::shared_ptr<BaseKalmanFilter> kalman_filter);
    virtual ~BaseTrack();

    virtual void update(Detection &det);
    virtual void predict();
    virtual cv::Rect2f getBox() const;
    virtual cv::Point2f getVelocity() const;

    static int getNextId() { return ++count; }
    void clearCount() { count = 0; }

    bool isActive() const { return state == TrackState::Tracked; }
    bool isLost() const { return state == TrackState::Lost; }
    bool isRemoved() const { return state == TrackState::Removed; }

    void markActive() { state = TrackState::Tracked; }
    void markLost() { state = TrackState::Lost; }
    void markRemoved() { state = TrackState::Removed; }
};

class BaseTracker
{
public:
    BaseTracker() = default;
    virtual ~BaseTracker() = default;
    virtual void update(std::vector<Detection> &detections) = 0;
    const std::vector<std::unique_ptr<BaseTrack>> &getTracks() const { return tracks; }

protected:
    std::vector<std::unique_ptr<BaseTrack>> tracks{};
};