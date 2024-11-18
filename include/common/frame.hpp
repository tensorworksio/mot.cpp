#pragma once

#include <memory>
#include <opencv2/opencv.hpp>
#include <common/detection.hpp>

struct Frame
{
    int idx = 0;
    cv::Mat image;
    std::vector<std::unique_ptr<Detection>> detections;
};
