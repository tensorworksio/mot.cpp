#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

struct Detection
{
    int frame;
    int id;
    cv::Rect2f bbox;
    float confidence;
    cv::Point3f position;

    cv::Scalar getColor() const
    {
        srand(id);
        return cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
    }

    friend std::istream &operator>>(std::istream &is, Detection &detection)
    {
        std::string field;

        std::getline(is, field, ',');
        detection.frame = std::stoi(field);

        std::getline(is, field, ',');
        detection.id = std::stoi(field);

        std::getline(is, field, ',');
        detection.bbox.x = std::stof(field);

        std::getline(is, field, ',');
        detection.bbox.y = std::stof(field);

        std::getline(is, field, ',');
        detection.bbox.width = std::stof(field);

        std::getline(is, field, ',');
        detection.bbox.height = std::stof(field);

        std::getline(is, field, ',');
        detection.confidence = std::stof(field);

        std::getline(is, field, ',');
        detection.position.x = std::stof(field);

        std::getline(is, field, ',');
        detection.position.y = std::stof(field);

        std::getline(is, field);
        detection.position.z = std::stof(field);

        return is;
    }

    friend std::ostream &operator<<(std::ostream &os, const Detection &detection)
    {
        os << detection.frame << "," << detection.id << ","
           << detection.bbox.x << "," << detection.bbox.y << ","
           << detection.bbox.width << "," << detection.bbox.height << ","
           << detection.confidence << ","
           << detection.position.x << "," << detection.position.y << "," << detection.position.z;
        return os;
    }
};
