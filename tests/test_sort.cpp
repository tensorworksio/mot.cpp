#include <gtest/gtest.h>
#include <tracking/sort.hpp>

class SortTest : public testing::Test
{
protected:
    void SetUp() override
    {
        BaseTrack::count = 0;
        config.max_time_lost = 3;
        config.match_thresh = 0.3f;
    }

    SortConfig config;

    static Detection makeDet(float x, float y, float w, float h, float conf = 0.9f)
    {
        Detection det;
        det.bbox = cv::Rect2f(x, y, w, h);
        det.confidence = conf;
        return det;
    }
};

TEST_F(SortTest, NoDetectionsNoTracks)
{
    Sort tracker(config);
    std::vector<Detection> dets;
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(SortTest, SingleDetectionCreatesOneTrack)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 1u);
}

TEST_F(SortTest, MultipleNonOverlappingDetectionsCreateMultipleTracks)
{
    Sort tracker(config);
    std::vector<Detection> dets = {
        makeDet(0, 0, 50, 50),
        makeDet(500, 500, 50, 50),
    };
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 2u);
}

TEST_F(SortTest, MatchedDetectionGetsTrackId)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);               // create track
    tracker.update(dets);               // match and assign track_id
    EXPECT_GT(dets[0].track_id, 0);
}

TEST_F(SortTest, PersistentDetectionDoesNotGrowTrackCount)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);
    size_t count_after_first = tracker.getTracks().size();

    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), count_after_first);
}

TEST_F(SortTest, UnmatchedTrackBecomesLost)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);

    std::vector<Detection> empty;
    tracker.update(empty);

    ASSERT_EQ(tracker.getTracks().size(), 1u);
    EXPECT_TRUE(tracker.getTracks()[0]->isLost());
}

TEST_F(SortTest, TrackRemovedAfterMaxTimeLost)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);

    std::vector<Detection> empty;
    // max_time_lost + 1 empty frames pushes time_since_update past the threshold
    for (size_t i = 0; i <= config.max_time_lost; ++i)
        tracker.update(empty);

    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(SortTest, DistantDetectionCreatesNewTrackWhileOldBecomesLost)
{
    Sort tracker(config);
    std::vector<Detection> dets1 = {makeDet(0, 0, 50, 50)};
    tracker.update(dets1);
    EXPECT_EQ(tracker.getTracks().size(), 1u);

    // Detection far away → zero IoU → no match → new track; old track marked lost
    std::vector<Detection> dets2 = {makeDet(1000, 1000, 50, 50)};
    tracker.update(dets2);
    EXPECT_EQ(tracker.getTracks().size(), 2u);
    // The original track must now be lost
    bool any_lost = false;
    for (const auto &t : tracker.getTracks())
        any_lost |= t->isLost();
    EXPECT_TRUE(any_lost);
}

TEST_F(SortTest, TrackAgeIncrementsEachFrame)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);

    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks()[0]->age, 1u);

    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks()[0]->age, 2u);
}

TEST_F(SortTest, MatchedTrackResetsTimeSinceUpdate)
{
    Sort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50)};
    tracker.update(dets);               // create

    std::vector<Detection> empty;
    tracker.update(empty);              // track goes lost, time_since_update = 1
    EXPECT_EQ(tracker.getTracks()[0]->time_since_update, 1u);

    tracker.update(dets);               // re-match, reset to 0
    EXPECT_EQ(tracker.getTracks()[0]->time_since_update, 0u);
}
