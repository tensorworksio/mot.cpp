#include <gtest/gtest.h>
#include <kalman/xywh.hpp>
#include <kalman/xysr.hpp>

// Derived classes override getBox(const cv::Mat&) and getVelocity(const cv::Mat&),
// which hides the no-arg base class methods. Access them through a base reference.

// --- KalmanFilterXYWH ---

class KalmanXYWHTest : public testing::Test
{
protected:
    cv::Rect2f rect{10.f, 20.f, 100.f, 50.f};
};

TEST_F(KalmanXYWHTest, InitializationMatchesRect)
{
    KalmanFilterXYWH kf(rect);
    BaseKalmanFilter &base = kf;
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 1.f);
    EXPECT_NEAR(box.y, rect.y, 1.f);
    EXPECT_NEAR(box.width, rect.width, 1.f);
    EXPECT_NEAR(box.height, rect.height, 1.f);
}

TEST_F(KalmanXYWHTest, InitialVelocityIsZero)
{
    KalmanFilterXYWH kf(rect);
    BaseKalmanFilter &base = kf;
    auto vel = base.getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.f);
    EXPECT_FLOAT_EQ(vel.y, 0.f);
}

TEST_F(KalmanXYWHTest, PredictReturnsValidBox)
{
    KalmanFilterXYWH kf(rect);
    // Zero velocity: prediction stays close to initial position
    auto pred = kf.predict();
    EXPECT_GE(pred.width, 0.f);
    EXPECT_GE(pred.height, 0.f);
    EXPECT_NEAR(pred.x, rect.x, 5.f);
    EXPECT_NEAR(pred.y, rect.y, 5.f);
}

TEST_F(KalmanXYWHTest, UpdateCorrectsTowardMeasurement)
{
    KalmanFilterXYWH kf(rect);
    BaseKalmanFilter &base = kf;
    kf.predict();
    kf.update(rect);
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 2.f);
    EXPECT_NEAR(box.y, rect.y, 2.f);
    EXPECT_NEAR(box.width, rect.width, 5.f);
    EXPECT_NEAR(box.height, rect.height, 5.f);
}

TEST_F(KalmanXYWHTest, ResetZerosVelocity)
{
    KalmanFilterXYWH kf(rect);
    BaseKalmanFilter &base = kf;
    kf.reset();
    auto vel = base.getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.f);
    EXPECT_FLOAT_EQ(vel.y, 0.f);
}

TEST_F(KalmanXYWHTest, GetBoxNonNegativeDimensions)
{
    KalmanFilterXYWH kf(cv::Rect2f(0.f, 0.f, 0.f, 0.f));
    BaseKalmanFilter &base = kf;
    auto box = base.getBox();
    EXPECT_GE(box.x, 0.f);
    EXPECT_GE(box.y, 0.f);
    EXPECT_GE(box.width, 0.f);
    EXPECT_GE(box.height, 0.f);
}

TEST_F(KalmanXYWHTest, WithCustomConfig)
{
    KalmanConfig config;
    config.process_noise_scale = 0.5f;
    config.measurement_noise_scale = 0.5f;

    KalmanFilterXYWH kf(rect, config);
    BaseKalmanFilter &base = kf;
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 1.f);
    EXPECT_NEAR(box.y, rect.y, 1.f);
    EXPECT_NEAR(box.width, rect.width, 1.f);
    EXPECT_NEAR(box.height, rect.height, 1.f);
}

TEST_F(KalmanXYWHTest, MultipleUpdatesConverge)
{
    KalmanFilterXYWH kf(rect);
    BaseKalmanFilter &base = kf;
    for (int i = 0; i < 5; ++i)
    {
        kf.predict();
        kf.update(rect);
    }
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 2.f);
    EXPECT_NEAR(box.y, rect.y, 2.f);
    EXPECT_NEAR(box.width, rect.width, 5.f);
    EXPECT_NEAR(box.height, rect.height, 5.f);
}

// --- KalmanFilterXYSR ---

class KalmanXYSRTest : public testing::Test
{
protected:
    cv::Rect2f rect{10.f, 20.f, 100.f, 50.f};
};

TEST_F(KalmanXYSRTest, InitializationMatchesRect)
{
    KalmanFilterXYSR kf(rect);
    BaseKalmanFilter &base = kf;
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 2.f);
    EXPECT_NEAR(box.y, rect.y, 2.f);
    EXPECT_NEAR(box.width, rect.width, 5.f);
    EXPECT_NEAR(box.height, rect.height, 5.f);
}

TEST_F(KalmanXYSRTest, InitialVelocityIsZero)
{
    KalmanFilterXYSR kf(rect);
    BaseKalmanFilter &base = kf;
    auto vel = base.getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.f);
    EXPECT_FLOAT_EQ(vel.y, 0.f);
}

TEST_F(KalmanXYSRTest, PredictReturnsValidBox)
{
    KalmanFilterXYSR kf(rect);
    auto pred = kf.predict();
    EXPECT_GE(pred.width, 0.f);
    EXPECT_GE(pred.height, 0.f);
    EXPECT_NEAR(pred.x, rect.x, 5.f);
    EXPECT_NEAR(pred.y, rect.y, 5.f);
}

TEST_F(KalmanXYSRTest, ResetZerosScaleVelocity)
{
    KalmanFilterXYSR kf(rect);
    BaseKalmanFilter &base = kf;
    kf.reset();
    auto vel = base.getVelocity();
    EXPECT_FLOAT_EQ(vel.x, 0.f);
    EXPECT_FLOAT_EQ(vel.y, 0.f);
}

TEST_F(KalmanXYSRTest, UpdateCorrectsTowardMeasurement)
{
    KalmanFilterXYSR kf(rect);
    BaseKalmanFilter &base = kf;
    kf.predict();
    kf.update(rect);
    auto box = base.getBox();
    EXPECT_NEAR(box.x, rect.x, 5.f);
    EXPECT_NEAR(box.y, rect.y, 5.f);
    EXPECT_NEAR(box.width, rect.width, 10.f);
    EXPECT_NEAR(box.height, rect.height, 10.f);
}
