#include <opencv2/highgui/highgui.hpp>

int main(int argc, char** argv) {
  cv::VideoCapture cap0(0);
  if (cap0.isOpened()) printf("camera successfully opened\n");
  else               { printf("cannot open camera\n"); return 1; }
  cv::namedWindow("image0", cv::WINDOW_AUTOSIZE);
  cv::Mat frame0;
  for ( ; ; ) {
    cap0 >> frame0;
    cv::imshow("image0", frame0);
    if (cv::waitKey(30) >= 0) break;
  }
  return 0;
}
