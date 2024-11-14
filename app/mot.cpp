#include <iostream>
#include <kalman/xywh.hpp>
#include <opencv2/opencv.hpp>

void printRect(cv::Rect2f &rect)
{
  std::cout << "x: " << rect.x << " y: " << rect.y << " w: " << rect.width << " h: " << rect.height << std::endl;
}

int main(int argc, char **argv)
{
  cv::Rect2f rect{100.f, 200.f, 300.f, 400.f};
  KalmanFilterXYWH kf(rect);

  auto position = kf.predict();
  printRect(position);

  cv::Rect2f rect2{103.f, 203.f, 300.f, 400.f};
  kf.update(rect2);

  position = kf.predict();
  printRect(position);
}
