#pragma once

#include <opencv2/opencv.hpp>

constexpr float EPSILON = 1e-6;

inline float iou(const cv::Rect2f &rect1, const cv::Rect2f &rect2)
{
    float in = (rect1 & rect2).area();
    float un = rect1.area() + rect2.area() - in;

    if (un < EPSILON)
        return 0.f;

    return in / un;
}