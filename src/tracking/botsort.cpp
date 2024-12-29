#include <tracking/botsort.hpp>
#include <utils/vector_utils.hpp>
#include <utils/geometry_utils.hpp>
#include <kalman/xywh.hpp>
#include <dlib/optimization/max_cost_assignment.h>

BotSortTrack::BotSortTrack(const cv::Rect2f &rect, const KalmanConfig &config) : BaseTrack(std::make_shared<KalmanFilterXYWH>(rect, config)) {}

BotSortTrack::BotSortTrack(const cv::Rect2f &rect, const std::vector<float> &feat, const KalmanConfig &config) : BaseTrack(std::make_shared<KalmanFilterXYWH>(rect, config)), features(feat) {}

void BotSortTrack::predict()
{
    if (!isActive())
        kf->reset();

    BaseTrack::predict();
}

void BotSortTrack::update(Detection &det)
{
    if (features.empty())
    {
        features = det.features;
    }
    else if (!det.features.empty())
    {
        features = vector_ops::normalize(vector_ops::compose(features, det.features, alpha));
    }
    BaseTrack::update(det);
}

void BotSort::assign(std::vector<Detection *> &dets,
                     std::vector<BotSortTrack *> &trks,
                     float match_thresh,
                     float proximity_thresh,
                     float appearance_thresh,
                     std::set<std::pair<size_t, size_t>> &matches,
                     std::set<size_t> &unmatched_detections,
                     std::set<size_t> &unmatched_tracks)
{

    // By default detections are unmatched
    for (size_t i = 0; i < dets.size(); ++i)
    {
        unmatched_detections.insert(i);
    }

    // By default tracks are unmatched
    for (size_t i = 0; i < trks.size(); ++i)
    {
        unmatched_tracks.insert(i);
    }

    if (trks.empty() || dets.empty())
        return;

    // Create cost matrix
    size_t size = std::max(dets.size(), trks.size());
    dlib::matrix<int> cost_matrix = dlib::zeros_matrix<int>(size, size);

    for (size_t i = 0; i < dets.size(); ++i)
    {
        for (size_t j = 0; j < trks.size(); ++j)
        {
            // Compute IoU similiarity
            float iou = getIoU(dets[i]->bbox, trks[j]->getBox());
            float un = (dets[i]->bbox | trks[j]->getBox()).area();
            float proximity = dets[i]->bbox.area() / un;

            // Compute cosine similarity
            float similiarity = 0.f;
            if (!dets[i]->features.empty() && !trks[j]->features.empty() && proximity > proximity_thresh)
            {
                similiarity = cosineSimilarity(dets[i]->features, trks[j]->features);
                similiarity = similiarity > appearance_thresh ? similiarity : 0.f;
            }

            cost_matrix(i, j) = static_cast<int>(PRECISION * std::max(iou, similiarity));
        }
    }

    // Solve linear assignment
    std::vector<long> assignment = dlib::max_cost_assignment(cost_matrix);

    // Find matches
    auto cost_thresh = static_cast<int>(PRECISION * match_thresh);
    for (size_t i = 0; i < dets.size(); ++i)
    {
        if (cost_matrix(i, assignment[i]) < cost_thresh)
            continue;

        unmatched_detections.erase(i);
        unmatched_tracks.erase(assignment[i]);
        matches.emplace(i, assignment[i]);
    }
}

void BotSort::update(std::vector<Detection> &detections)
{
    // Detection bins
    std::vector<Detection *> high_score_detections{};
    std::vector<Detection *> low_score_detections{};
    std::vector<Detection *> unconfirmed_detections{};

    for (auto &det : detections)
    {
        if (det.confidence >= config.track_high_thresh)
            high_score_detections.push_back(&det);
        else if (det.confidence >= config.track_low_thresh)
            low_score_detections.push_back(&det);
        else
            unconfirmed_detections.push_back(&det);
    }

    // Track bins
    std::vector<BotSortTrack *> lost_tracks{};
    std::vector<BotSortTrack *> active_tracks{};
    std::vector<BotSortTrack *> unmatched_tracks{};
    std::vector<BotSortTrack *> unconfirmed_tracks{};

    for (auto &track : tracks)
    {
        auto *bot_track = dynamic_cast<BotSortTrack *>(track.get());
        if (!bot_track)
            continue;
        else if (bot_track->isActive())
            active_tracks.push_back(bot_track);
        else if (bot_track->isLost())
        {
            lost_tracks.push_back(bot_track);
            active_tracks.push_back(bot_track);
        }
        else
            unconfirmed_tracks.push_back(bot_track);
    }

    // Propagate tracks
    for (auto &track : tracks)
    {
        track->predict();
    }

    // First association
    std::set<std::pair<size_t, size_t>> first_matches;
    std::set<size_t> first_unmatched_detections;
    std::set<size_t> first_unmatched_tracks;

    assign(high_score_detections,
           active_tracks,
           config.first_match_thresh,
           config.proximity_thresh,
           config.appearance_thresh,
           first_matches,
           first_unmatched_detections,
           first_unmatched_tracks);

    for (const auto &[det_idx, track_idx] : first_matches)
    {
        auto *det = high_score_detections[det_idx];
        auto *track = active_tracks[track_idx];
        track->update(*det);
        det->id = track->id;
    }

    for (const auto &track_idx : first_unmatched_tracks)
    {
        auto *track = active_tracks[track_idx];
        if (track->isActive())
            unmatched_tracks.push_back(track);
    }

    for (const auto &det_idx : first_unmatched_detections)
    {
        auto *det = high_score_detections[det_idx];
        unconfirmed_detections.push_back(det);
    }

    // Second association
    std::set<std::pair<size_t, size_t>> second_matches;
    std::set<size_t> second_unmatched_detections;
    std::set<size_t> second_unmatched_tracks;

    assign(low_score_detections,
           unmatched_tracks,
           config.second_match_thresh,
           0.f,
           1.f,
           second_matches,
           second_unmatched_detections,
           second_unmatched_tracks);

    for (const auto &[det_idx, track_idx] : second_matches)
    {
        auto *det = low_score_detections[det_idx];
        auto *track = unmatched_tracks[track_idx];
        track->update(*det);
        det->id = track->id;
    }

    for (const auto &track_idx : second_unmatched_tracks)
    {
        auto *track = unmatched_tracks[track_idx];
        track->markLost();
        lost_tracks.push_back(track);
    }

    for (const auto &det_idx : second_unmatched_detections)
    {
        auto *det = low_score_detections[det_idx];
        unconfirmed_detections.push_back(det);
    }

    // Handle unconfirmed tracks
    std::set<std::pair<size_t, size_t>> unconfirmed_matches;
    std::set<size_t> unconfirmed_unmatched_detections;
    std::set<size_t> unconfirmed_unmatched_tracks;

    assign(unconfirmed_detections,
           unconfirmed_tracks,
           config.unconfirmed_match_thresh,
           config.proximity_thresh,
           config.appearance_thresh,
           unconfirmed_matches,
           unconfirmed_unmatched_detections,
           unconfirmed_unmatched_tracks);

    for (const auto &[det_idx, track_idx] : unconfirmed_matches)
    {
        auto *det = unconfirmed_detections[det_idx];
        auto *track = unconfirmed_tracks[track_idx];
        track->update(*det);
        det->id = track->id;
    }

    for (const auto &track_idx : unconfirmed_unmatched_tracks)
    {
        auto *track = unconfirmed_tracks[track_idx];
        track->markRemoved();
    }

    // Initialize new tracks
    for (const auto &det_idx : unconfirmed_unmatched_detections)
    {
        auto *det = unconfirmed_detections[det_idx];
        if (det->confidence > config.new_track_thresh)
        {
            auto new_track = std::make_unique<BotSortTrack>(det->bbox, det->features, config.kalman);
            tracks.push_back(std::move(new_track));
        }
    }

    // Remove old tracks
    for (auto &track : lost_tracks)
    {
        if (track->time_since_update > config.max_time_lost)
            track->markRemoved();
    }

    tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const auto &track)
                                { return track->isRemoved(); }),
                 tracks.end());
}