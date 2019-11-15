#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

bool calib(cv::Mat &frame, const cv::Size ZZ, int picture, std::vector<std::vector<cv::Point2f> > &points, float SS,
           cv::Mat &MM, cv::Mat &DD, std::vector<cv::Mat> &rvecs, std::vector<cv::Mat> &tvecs, double &rms);
void map12_make(cv::Mat frame, cv::Mat MM, cv::Mat DD, cv::Mat &map1, cv::Mat &map2);

int main(int argc, char** argv) {
  if (argc == 5) {
    const cv::Size ZZ = cv::Size(atoi(argv[1]), atoi(argv[2]));
    float SS = atof(argv[3]);
    cv::Mat frame = cv::imread(argv[4]);
    cv::Mat MM;
    cv::Mat DD;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms;
    cv::Mat map1;
    cv::Mat map2;
    cv::Mat nframe;
    std::vector<std::vector<cv::Point2f> > points;
    cv::namedWindow("board");
    if (calib(frame, ZZ, 0, points, SS, MM, DD, rvecs, tvecs, rms)) {
      printf("rms=%g\n", rms);
      printf("(fx fy cx cy)=(%g %g %g %g)\n",
      MM.at<double>(0, 0), MM.at<double>(1, 1), MM.at<double>(0, 2), MM.at<double>(1, 2));
      printf("(k1 k2 p1 p2 k3)=(%g %g %g %g %g)\n",
      DD.at<double>(0, 0), DD.at<double>(0, 1), DD.at<double>(0, 2), DD.at<double>(0, 3), DD.at<double>(0, 4));
      printf("rvecs=(%g %g %g)\n", rvecs[0].at<double>(0, 0), rvecs[0].at<double>(0, 1), rvecs[0].at<double>(0, 2));
      printf("tvecs=(%g %g %g)\n", tvecs[0].at<double>(0, 0), tvecs[0].at<double>(0, 1), tvecs[0].at<double>(0, 2));
      map12_make(frame, MM, DD, map1, map2);
      cv::remap(frame, nframe, map1, map2, cv::INTER_LINEAR);
      cv::imshow("board", nframe);
      int key = cv::waitKey(0);
      if (key == 'w') cv::imwrite("undistort.png", nframe);
    }
    return 0;
  }
  else {
    printf("Usage: './program_name num_of_cols num_of_rows distance image.png'\n");
    return 1;
  }
}
