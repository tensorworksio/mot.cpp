#include <tracker/sort.hpp>
#include <common/metrics.hpp>
#include <dlib/optimization/max_cost_assignment.h>

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
            cost_matrix(i, j) = static_cast<int>(PRECISION * iou(detections[i].bbox, tracks[j]->getBox()));
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

void process(Frame &frame)
{
    return;
}