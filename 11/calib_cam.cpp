#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

bool calib(cv::Mat &frame, const cv::Size ZZ, int picture, std::vector<std::vector<cv::Point2f> > &points, float SS,
           cv::Mat &MM, cv::Mat &DD, std::vector<cv::Mat> &rvecs, std::vector<cv::Mat> &tvecs, double &rms);
void map12_make(cv::Mat frame, cv::Mat MM, cv::Mat DD, cv::Mat &map1, cv::Mat &map2);
void map12_init(cv::Mat &map1, cv::Mat &map2);

int main(int argc, char** argv) {
  if (argc == 4) {
    const cv::Size ZZ = cv::Size(atoi(argv[1]), atoi(argv[2]));
    float SS = atof(argv[3]);
    cv::Mat MM;
    cv::Mat DD;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms;
    cv::Mat map1;
    cv::Mat map2;
    cv::Mat frame;
    cv::Mat nframe;
    cv::Mat bframe;
    int picture = 0;
    std::vector<std::vector<cv::Point2f> > points;
    map12_init(map1, map2);
    cv::VideoCapture cap(2);
    if (cap.isOpened()) printf("camera successfully opened\n");
    else              { printf("cannot open camera\n"); return 1; }
    cv::namedWindow("board");
    for ( ; ; ) {
      cap >> frame;
      cv::remap(frame, nframe, map1, map2, cv::INTER_LINEAR);
      cv::imshow("board", nframe);
      int key = cv::waitKey(30);
      if (key == 'q') break;
      else if (key == 'd') {
        map12_init(map1, map2);
        picture = 0;
      }
      else if (key == 'o') {
        cv::imwrite("board_ok.png", bframe);
        printf("saved previous frame as 'board_ok.png'\n");
      }
      else if (key == 'n') {
        cv::imwrite("board_ng.png", bframe);
        printf("saved previous frame as 'board_ng.png'\n");
      }
      else if (key == 'c') {
        bframe = frame.clone();
        if (calib(frame, ZZ, picture, points, SS, MM, DD, rvecs, tvecs, rms)) {
          printf("use %d pictures\t", picture+1);
          printf("rms=%g\t", rms);
          printf("focal_u=%g\t", MM.at<double>(0, 0));
          printf("focal_v=%g\n", MM.at<double>(1, 1));
          map12_make(frame, MM, DD, map1, map2);
          cv::remap(frame, nframe, map1, map2, cv::INTER_LINEAR);
          cv::imshow("board", nframe);
          cv::waitKey(500);
          picture++;
        }
      }
      else if (key == 's') {
        cv::FileStorage fs("camera.yml", cv::FileStorage::WRITE);
        fs << "camera_matrix" << MM << "distortion_coefficients" << DD;
        fs.release();
        printf("saved MM and DD to 'camera.yml'\n");
      }
      else ;
    }
    return 0;
  }
  else {
    printf("Usage: './program_name num_of_cols num_of_rows distance'\n");
    return 1;
  }
}
