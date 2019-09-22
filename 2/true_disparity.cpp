/********************************************************************/
/***** make true disparity file *************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.15 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

#define W_SIZE (SCALE*2*(XX-1)+ZZ+1)
#define H_SIZE (YY)
#define BUF_NUM (W_SIZE*H_SIZE)
#define BUF_ADR(H, W) \
 ((H)*W_SIZE+\
  (W))
struct position {
  int x;
  int y;
  int dis;
};

void dis_to_dis(const char *i_name, const char *o_name, int SCALE) {
  cv::Mat frame = cv::imread(i_name, 0);
  if (frame.empty()) {
    printf("cannot open %s\n", i_name);
    return;
  }
  int XX = frame.cols;
  int YY = frame.rows;
  int ZZ = 0;
  int WW = 256;
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int dis = frame.at<uchar>(y, x);
      if (dis > ZZ) ZZ = dis;
      if (dis > 0) if (dis < WW) WW = dis;
    }
  }
  printf("XX=%d YY=%d max(scaled)disparity=%g(%d) min(scaled)disparite=%g(%d)\n", XX, YY, (double)ZZ/SCALE, ZZ, (double)WW/SCALE, WW);
  position *buf = new position[BUF_NUM];
  for (int i = 0; i < BUF_NUM; i++) buf[i].dis = 0;
  /***** translate from pixel-disparity to gaze_line-depth *****/
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int dis = frame.at<uchar>(y, x);
      if (dis > 0) {
        ///////////////////////////////////////////////////////
        int W = SCALE*2*x + dis;
        ///////////////////////////////////////////////////////
        int p = buf[BUF_ADR(y, W)].dis;
        if (dis > p) {
          buf[BUF_ADR(y, W)].x = x;
          buf[BUF_ADR(y, W)].y = y;
          buf[BUF_ADR(y, W)].dis = dis;
        }
      }
    }
  }
  /***** remove contradiction *****/
  for (int H = 0; H < H_SIZE; H++) {
    for (int W = 0; W < W_SIZE; W++) {
      int S = buf[BUF_ADR(H, W)].dis;
      if (S != 0) {
        int r = 1;
        for ( ; ; ) {
          int RW = W + r;
          if (RW == W_SIZE) break;
          int SS = buf[BUF_ADR(H, RW)].dis;
          if (SS != 0) {
            if (SS <= S-r) buf[BUF_ADR(H, RW)].dis = 0;
          }
          r++;
        }
        int l = 1;
        for ( ; ; ) {
          int LW = W - l;
          if (LW == -1) break;
          int SS = buf[BUF_ADR(H, LW)].dis;
          if (SS != 0) {
            if (SS <= S-l) buf[BUF_ADR(H, LW)].dis = 0;
          }
          l++;
        }
      }
    }
  }
  /***** make true disparity *****/
  cv::Mat truth(YY, XX, CV_8U, cv::Scalar(0));
  for (int H = 0; H < H_SIZE; H++) {
    for (int W = 0; W < W_SIZE; W++) {
      if (buf[BUF_ADR(H, W)].dis != 0) {
        int x = buf[BUF_ADR(H, W)].x;
        int y = buf[BUF_ADR(H, W)].y;
        int dis = buf[BUF_ADR(H, W)].dis;
        if (x < XX-WW/SCALE) truth.at<uchar>(y, x) = dis;
      }
    }
  }
  cv::imwrite(o_name, truth);
}

/********************************************************************/
int main(int argc, const char** argv) {
  if (argc == 4) {
    dis_to_dis(argv[1], argv[2], atoi(argv[3]));
    return 0;
  }
  else {
    printf("Usage: './true input_disparity_file output_true_disparity_file scale_factor'\n");
    return 1;
  }
}
