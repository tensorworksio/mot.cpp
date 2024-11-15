#pragma once

#include <opencv2/opencv.hpp>
#include <common/detection.hpp>

struct Frame
{
    int idx = 0;
    cv::Mat image;
    std::vector<Detection> detections;
};
