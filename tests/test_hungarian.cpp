#include <gtest/gtest.h>
#include <assignment/hungarian.hpp>

static float totalCost(const cv::Mat_<float>& cost, const std::vector<long>& assignment)
{
    float total = 0.f;
    for (int i = 0; i < static_cast<int>(assignment.size()); ++i)
        total += cost(i, assignment[i]);
    return total;
}

TEST(HungarianTest, EmptyMatrixReturnsEmpty)
{
    cv::Mat_<float> cost(0, 0);
    EXPECT_TRUE(hungarian::max_cost_assignment(cost).empty());
}

TEST(HungarianTest, SingleElement)
{
    cv::Mat_<float> cost(1, 1, 7.f);
    auto assignment = hungarian::max_cost_assignment(cost);
    ASSERT_EQ(assignment.size(), 1u);
    EXPECT_EQ(assignment[0], 0);
}

TEST(HungarianTest, TwoByTwoDiagonalPreferred)
{
    cv::Mat_<float> cost(2, 2);
    cost(0, 0) = 0.9f; cost(0, 1) = 0.1f;
    cost(1, 0) = 0.1f; cost(1, 1) = 0.8f;

    auto assignment = hungarian::max_cost_assignment(cost);
    ASSERT_EQ(assignment.size(), 2u);
    EXPECT_EQ(assignment[0], 0);
    EXPECT_EQ(assignment[1], 1);
}

TEST(HungarianTest, TwoByTwoOffDiagonalPreferred)
{
    cv::Mat_<float> cost(2, 2);
    cost(0, 0) = 0.1f; cost(0, 1) = 0.9f;
    cost(1, 0) = 0.8f; cost(1, 1) = 0.1f;

    auto assignment = hungarian::max_cost_assignment(cost);
    ASSERT_EQ(assignment.size(), 2u);
    EXPECT_EQ(assignment[0], 1);
    EXPECT_EQ(assignment[1], 0);
}

// 3x3 with a unique optimal: row0→col0(4), row1→col2(5), row2→col1(2) = 11
// All other permutations sum to less.
TEST(HungarianTest, ThreeByThreeKnownOptimal)
{
    cv::Mat_<float> cost(3, 3);
    cost(0, 0) = 4.f; cost(0, 1) = 1.f; cost(0, 2) = 3.f;
    cost(1, 0) = 2.f; cost(1, 1) = 0.f; cost(1, 2) = 5.f;
    cost(2, 0) = 3.f; cost(2, 1) = 2.f; cost(2, 2) = 2.f;

    auto assignment = hungarian::max_cost_assignment(cost);
    ASSERT_EQ(assignment.size(), 3u);
    EXPECT_NEAR(totalCost(cost, assignment), 11.f, 1e-4f);
    EXPECT_EQ(assignment[0], 0);
    EXPECT_EQ(assignment[1], 2);
    EXPECT_EQ(assignment[2], 1);
}

// Mirrors the MOT use case: 2 detections vs 3 tracks → 3x3 padded with a zero row.
// Optimal: row0→col0, row1→col1, padded row→col2.
TEST(HungarianTest, PaddedZeroRowHandledCorrectly)
{
    cv::Mat_<float> cost(3, 3, 0.f);
    cost(0, 0) = 0.8f; cost(0, 1) = 0.1f;
    cost(1, 0) = 0.1f; cost(1, 1) = 0.9f;
    // row 2 stays zero (padding)

    auto assignment = hungarian::max_cost_assignment(cost);
    ASSERT_EQ(assignment.size(), 3u);
    EXPECT_NEAR(totalCost(cost, assignment), 1.7f, 1e-4f);
    EXPECT_EQ(assignment[0], 0);
    EXPECT_EQ(assignment[1], 1);
}
