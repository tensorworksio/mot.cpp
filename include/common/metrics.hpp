#pragma once

#include <opencv2/opencv.hpp>
#include <utils/vector_utils.hpp>

constexpr float EPSILON = 1e-6;

inline float getIoU(const cv::Rect2f &rect1, const cv::Rect2f &rect2)
{
    float in = (rect1 & rect2).area();
    float un = rect1.area() + rect2.area() - in;

    if (un < EPSILON)
        return 0.f;

    return in / un;
}

inline float cosineSimilarity(const std::vector<float> &vec1, const std::vector<float> &vec2)
{
    float similarity;
    float dotProduct = vector_ops::dot(vec1, vec2);
    float normVec1 = std::sqrt(vector_ops::dot(vec1, vec1));
    float normVec2 = std::sqrt(vector_ops::dot(vec2, vec2));

    if (normVec1 == 0.f || normVec2 == 0.f)
    {
        return 0.f;
    }

    similarity = (1.f + dotProduct / (normVec1 * normVec2)) / 2.f;
    return similarity;
}