#include <gtest/gtest.h>
#include <cmath>
#include <tracking/botsort.hpp>

// --- BotSortTrack unit tests ---

class BotSortTrackTest : public testing::Test
{
protected:
    void SetUp() override { BaseTrack::count = 0; }

    cv::Rect2f rect{10.f, 20.f, 100.f, 50.f};
    KalmanConfig config{};

    static Detection makeDet(cv::Rect2f bbox, float conf, std::vector<float> feat = {})
    {
        Detection det;
        det.bbox = bbox;
        det.confidence = conf;
        det.features = feat;
        return det;
    }
};

TEST_F(BotSortTrackTest, InitWithoutFeaturesHasEmptyFeatures)
{
    BotSortTrack track(rect, config);
    EXPECT_TRUE(track.features.empty());
}

TEST_F(BotSortTrackTest, InitWithFeaturesStoresFeatures)
{
    std::vector<float> feat = {1.f, 0.f, 0.f};
    BotSortTrack track(rect, feat, config);
    EXPECT_EQ(track.features, feat);
}

TEST_F(BotSortTrackTest, FirstUpdateAssignsFeaturesDirectly)
{
    BotSortTrack track(rect, config);
    std::vector<float> feat = {1.f, 0.f, 0.f};
    auto det = makeDet(rect, 0.9f, feat);
    track.update(det);
    EXPECT_EQ(track.features, feat);
}

TEST_F(BotSortTrackTest, SubsequentUpdateAppliesEMAAndNormalization)
{
    // alpha = 0.9: new = normalize(0.9 * old + 0.1 * incoming)
    std::vector<float> feat1 = {1.f, 0.f};
    BotSortTrack track(rect, feat1, config);

    std::vector<float> feat2 = {0.f, 1.f};
    auto det = makeDet(rect, 0.9f, feat2);
    track.update(det);

    // Expected: normalize({0.9, 0.1})
    float norm = std::sqrt(0.81f + 0.01f);
    ASSERT_EQ(track.features.size(), 2u);
    EXPECT_NEAR(track.features[0], 0.9f / norm, 1e-5f);
    EXPECT_NEAR(track.features[1], 0.1f / norm, 1e-5f);
}

TEST_F(BotSortTrackTest, UpdateWithNoDetectionFeaturesKeepsOldFeatures)
{
    std::vector<float> feat = {1.f, 0.f};
    BotSortTrack track(rect, feat, config);

    auto det = makeDet(rect, 0.9f, {});   // no features
    track.update(det);

    EXPECT_EQ(track.features, feat);
}

TEST_F(BotSortTrackTest, UpdateMarksTrackActive)
{
    BotSortTrack track(rect, config);
    auto det = makeDet(rect, 0.9f);
    track.update(det);
    EXPECT_TRUE(track.isActive());
}

TEST_F(BotSortTrackTest, UpdateResetsTimeSinceUpdate)
{
    BotSortTrack track(rect, config);
    track.predict();   // increments time_since_update to 1

    auto det = makeDet(rect, 0.9f);
    track.update(det);
    EXPECT_EQ(track.time_since_update, 0u);
}

// --- BotSort tracker integration tests ---

class BotSortTest : public testing::Test
{
protected:
    void SetUp() override
    {
        BaseTrack::count = 0;
        config.max_time_lost = 3;
        config.track_high_thresh = 0.5f;
        config.track_low_thresh = 0.1f;
        config.new_track_thresh = 0.6f;
        config.first_match_thresh = 0.3f;
        config.second_match_thresh = 0.1f;
        config.unconfirmed_match_thresh = 0.2f;
    }

    BotSortConfig config;

    static Detection makeDet(float x, float y, float w, float h, float conf = 0.9f)
    {
        Detection det;
        det.bbox = cv::Rect2f(x, y, w, h);
        det.confidence = conf;
        return det;
    }
};

TEST_F(BotSortTest, NoDetectionsNoTracks)
{
    BotSort tracker(config);
    std::vector<Detection> dets;
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(BotSortTest, HighConfidenceDetectionCreatesTrack)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 1u);
}

TEST_F(BotSortTest, DetectionBelowNewTrackThreshDoesNotCreateTrack)
{
    BotSort tracker(config);
    // confidence is above track_high_thresh (0.5) but below new_track_thresh (0.6)
    // → goes through first association as unmatched, then unconfirmed association,
    //   but confidence check at track creation fails
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.55f)};
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(BotSortTest, VeryLowConfidenceDoesNotCreateTrack)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.05f)};
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(BotSortTest, NewTrackRemovedIfNoMatchNextFrame)
{
    // Unconfirmed tracks (New state) are removed immediately on the next frame if unmatched
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 1u);

    std::vector<Detection> empty;
    tracker.update(empty);
    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(BotSortTest, MatchedNewTrackBecomesActive)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);   // New track created
    tracker.update(dets);   // Matched via unconfirmed association → Active

    ASSERT_EQ(tracker.getTracks().size(), 1u);
    EXPECT_TRUE(tracker.getTracks()[0]->isActive());
}

TEST_F(BotSortTest, PersistentDetectionDoesNotGrowTrackCount)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);
    tracker.update(dets);   // activate
    size_t count = tracker.getTracks().size();

    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), count);
}

TEST_F(BotSortTest, ActiveTrackBecomesLostWhenUnmatched)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);   // New
    tracker.update(dets);   // Active

    std::vector<Detection> empty;
    tracker.update(empty);  // no match → Lost

    ASSERT_EQ(tracker.getTracks().size(), 1u);
    EXPECT_TRUE(tracker.getTracks()[0]->isLost());
}

TEST_F(BotSortTest, ActiveTrackRemovedAfterMaxTimeLost)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {makeDet(10, 20, 100, 50, 0.9f)};
    tracker.update(dets);   // New
    tracker.update(dets);   // Active

    std::vector<Detection> empty;
    for (size_t i = 0; i <= config.max_time_lost; ++i)
        tracker.update(empty);

    EXPECT_EQ(tracker.getTracks().size(), 0u);
}

TEST_F(BotSortTest, MultipleHighConfDetectionsCreateMultipleTracks)
{
    BotSort tracker(config);
    std::vector<Detection> dets = {
        makeDet(0, 0, 50, 50, 0.9f),
        makeDet(500, 500, 50, 50, 0.9f),
    };
    tracker.update(dets);
    EXPECT_EQ(tracker.getTracks().size(), 2u);
}
