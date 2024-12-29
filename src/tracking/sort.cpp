#include <tracking/sort.hpp>
#include <kalman/xywh.hpp>
#include <utils/geometry_utils.hpp>
#include <dlib/optimization/max_cost_assignment.h>

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

void Sort::assign(std::vector<Detection> &detections,
                  float match_thresh,
                  std::set<std::pair<size_t, size_t>> &matches,
                  std::set<size_t> &unmatched_detections,
                  std::set<size_t> &unmatched_tracks)
{

    // By default detections are unmatched
    for (size_t i = 0; i < detections.size(); ++i)
    {
        unmatched_detections.insert(i);
    }

    // By default tracks are unmatched
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        unmatched_tracks.insert(i);
    }

    if (tracks.empty() || detections.empty())
    {
        return;
    }

    // Create cost matrix
    size_t size = std::max(detections.size(), tracks.size());
    dlib::matrix<int> cost_matrix = dlib::zeros_matrix<int>(size, size);
    for (size_t i = 0; i < detections.size(); ++i)
    {
        for (size_t j = 0; j < tracks.size(); ++j)
        {
            cost_matrix(i, j) = static_cast<int>(PRECISION * getIoU(detections[i].bbox, tracks[j]->getBox()));
        }
    }

    // Solve linear assignment
    std::vector<long> assignment = dlib::max_cost_assignment(cost_matrix);

    // Find matches
    auto cost_thresh = static_cast<int>(PRECISION * match_thresh);
    for (size_t i = 0; i < detections.size(); ++i)
    {
        if (cost_matrix(i, assignment[i]) < cost_thresh)
            continue;

        unmatched_detections.erase(i);
        unmatched_tracks.erase(assignment[i]);
        matches.emplace(i, assignment[i]);
    }
}

void Sort::update(std::vector<Detection> &detections)
{
    std::set<std::pair<size_t, size_t>> matches;
    std::set<size_t> unmatched_detections;
    std::set<size_t> unmatched_tracks;

    // Propagate tracks
    for (auto &track : tracks)
    {
        track->predict();
    }

    // Assign detections to tracks
    assign(detections, config.match_thresh, matches, unmatched_detections, unmatched_tracks);

    // Update tracks
    for (const auto &[det_idx, track_idx] : matches)
    {
        tracks[track_idx]->update(detections[det_idx]);
        detections[det_idx].id = tracks[track_idx]->id;
    }

    // Create new tracks
    for (const auto &det_idx : unmatched_detections)
    {
        auto new_track = std::make_unique<SortTrack>(detections[det_idx].bbox, config.kalman);
        tracks.push_back(std::move(new_track));
    }

    for (const auto &track_idx : unmatched_tracks)
    {
        if (tracks[track_idx]->time_since_update > config.max_time_lost)
        {
            tracks[track_idx]->markRemoved();
        }
        else
        {
            tracks[track_idx]->markLost();
        }
    }

    // Remove tracks
    tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const auto &track)
                                { return track->isRemoved(); }),
                 tracks.end());
}