/********************************************************************/
/***** compare disparity ********************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.15 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

void compare_disparity(const char *t_name, const char *p_name) {
  cv::Mat t_frame = cv::imread(t_name, 0);
  if (t_frame.empty()) {
    printf("cannot open %s\n", t_name);
    return;
  }
  cv::Mat p_frame = cv::imread(p_name, 0);
  if (p_frame.empty()) {
    printf("cannot open %s\n", p_name);
    return;
  }
  if ((t_frame.cols != p_frame.cols)
    ||(t_frame.rows != p_frame.rows)) {
    printf("Two images's sizes are not equal.\n");
    return;
  }
  int XX = t_frame.cols;
  int YY = t_frame.rows;
  int MAX = 0;
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int T = t_frame.at<uchar>(y, x);
      if (T > 0) {
        int P = p_frame.at<uchar>(y, x);
        int D = (T>P)? T-P: P-T;
        if (D > MAX) MAX = D;
      }
    }
  }
  int count = 0;
  int eval[MAX+1] = {};
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int T = t_frame.at<uchar>(y, x);
      if (T > 0) {
        count++;
        int P = p_frame.at<uchar>(y, x);
        int D = (T>P)? T-P: P-T;
        eval[D]++;
      }
    }
  }
//for (int i = 0; i <= MAX; i++) if (eval[i]) printf("eval[%d]=%d(%d%%)\n", i, eval[i], 100*eval[i]/count);
  int total = 0;
  for (int i = 0; i <= MAX; i++) total += i*eval[i];
  printf("total=%d\n", total);
}

/********************************************************************/
int main(int argc, const char** argv) {
  if (argc == 3) {
    compare_disparity(argv[1], argv[2]);
    return 0;
  }
  else {
    printf("Usage: './compare true_disparity_file disparity_file'\n");
    return 1;
  }
}
