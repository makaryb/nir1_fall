#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

bool calib(cv::Mat &frame, const cv::Size ZZ, int picture, std::vector<std::vector<cv::Point2f> > &points, float SS,
           cv::Mat &MM, cv::Mat &DD, std::vector<cv::Mat> &rvecs, std::vector<cv::Mat> &tvecs, double &rms) {
  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  std::vector<cv::Point2f> corners;
  bool found = cv::findCirclesGrid(gray, ZZ, corners);
  if (!found) {
    found = cv::findChessboardCorners(frame, ZZ, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
    if (!found) { printf("cannot find calibration board\n"); return false; }
    cv::cornerSubPix(gray, corners, cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 30, 0.1));
  }
  cv::drawChessboardCorners(frame, ZZ, corners, found);
  /*---------------------------*/
  if (picture == 0) points.clear();
  points.push_back(corners);
  /*---------------------------*/
  std::vector<cv::Point3f> Corners;
  for (int h = 0; h < ZZ.height; h++) {
    for (int w = 0; w < ZZ.width; w++) {
      Corners.push_back(cv::Point3f(w*SS, h*SS, 0.0f));
    }
  }
  /*---------------------------*/
  std::vector<std::vector<cv::Point3f> > Points;
  for (int i = 0; i <= picture; i++) {
    Points.push_back(Corners);
  }
  /*---------------------------*/
  rms = cv::calibrateCamera(Points, points, frame.size(), MM, DD, rvecs, tvecs,
//CV_CALIB_USE_INTRINSIC_GUESS+
//CV_CALIB_FIX_PRINCIPAL_POINT+
//CV_CALIB_FIX_ASPECT_RATIO+
//CV_CALIB_ZERO_TANGENT_DIST+
//CV_CALIB_FIX_K1+
//CV_CALIB_FIX_K2+
//CV_CALIB_FIX_K3+
//CV_CALIB_FIX_K4+
//CV_CALIB_FIX_K5+
//CV_CALIB_FIX_K6+
//CV_CALIB_RATIONAL_MODEL+
//CV_CALIB_THIN_PRISM_MODEL+
//CV_CALIB_FIX_S1_S2_S3_S4+
//CV_CALIB_TILTED_MODEL+
//CV_CALIB_FIX_TAUX_TAUY+
  0);
  return true;
}

void map12_make(cv::Mat frame, cv::Mat MM, cv::Mat DD, cv::Mat &map1, cv::Mat &map2) {
  cv::Mat NN = cv::getOptimalNewCameraMatrix(MM, DD, frame.size(), 1, frame.size());
  cv::Mat VV = cv::Mat::zeros(1, 3, CV_64F);
  cv::Mat RR;
  cv::Rodrigues(VV, RR);
  cv::initUndistortRectifyMap(MM, DD, RR, NN, frame.size(), CV_16SC2, map1, map2);
}

void map12_init(cv::Mat &map1, cv::Mat &map2) {
  cv::Mat MM = cv::Mat::zeros(3, 3, CV_64F);
  MM.at<double>(0, 0) = 800.0f;
  MM.at<double>(0, 2) = double(640-1)/2;
  MM.at<double>(1, 1) = 800.0f;
  MM.at<double>(1, 2) = double(480-1)/2;
  MM.at<double>(2, 2) = 1.0;
  cv::Mat DD = cv::Mat::zeros(1, 5, CV_64F);
  cv::Mat VV = cv::Mat::zeros(1, 3, CV_64F);
  cv::Mat RR;
  cv::Rodrigues(VV, RR);
  cv::initUndistortRectifyMap(MM, DD, RR, MM, cv::Size(640, 480), CV_16SC2, map1, map2);
}
