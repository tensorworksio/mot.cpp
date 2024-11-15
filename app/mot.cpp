#include <iostream>
#include <tracker/sort.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv)
{
    SortConfig config;
    Sort tracker(config);

    std::cout << config.match_thresh << std::endl;
    std::cout << config.max_time_lost << std::endl;
}
