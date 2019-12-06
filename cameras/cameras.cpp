#include <opencv2/highgui/highgui.hpp>

int main(int argc, const char** argv) {
  cv::VideoCapture cap0(0);
  cv::VideoCapture cap1(1);
  if (cap0.isOpened()) printf("right camera successfully opened\n");
  else               { printf("cannot open right camera\n"); return 1; }
  if (cap1.isOpened()) printf("left camera successfully opened\n");
  else               { printf("cannot open left camera\n"); return 1; }
  cv::namedWindow("image0", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("image1", cv::WINDOW_AUTOSIZE);
  cv::Mat frame0;
  cv::Mat frame1;
  for ( ; ; ) {
    cap0 >> frame0;
    cap1 >> frame1;
    cv::imshow("image0", frame0);
    cv::imshow("image1", frame1);
    if (cv::waitKey(30) >= 0) break;
  }
  return 0;
}
