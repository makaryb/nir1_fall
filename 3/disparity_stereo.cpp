/********************************************************************/
/***** Stereo Vision using Pixel-Disparity Model ********************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

extern void graph_cut(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b);
extern int SIZE_S;
extern int SIZE_H;
extern int SIZE_W;
extern int SIZE_HW;
extern int SIZE_SHW;
int Max_depth;
int Min_depth;

#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

cv::Mat rframe;
cv::Mat lframe;

int *DEP;

#define IADR(H, W) \
 ((H)*SIZE_W+\
  (W))

void call_rgb(cv::Mat &frame, int y, int x, int *r, int *g, int *b) {
  cv::Vec3b bgr = frame.at<cv::Vec3b>(y,x);
  *b = bgr[0]; *g = bgr[1]; *r = bgr[2];
}

int cost(int D, int H, int W) {
  D += Min_depth;
  int RR, RG, RB;
  int LR, LG, LB;
  int LW;
  if (W+D > SIZE_W-1) LW = SIZE_W-1;
  else                LW = W+D;
  call_rgb(rframe, H, W, &RR, &RG, &RB);
  call_rgb(lframe, H, LW, &LR, &LG, &LB);
  int d_R = (LR>RR)? LR-RR: RR-LR;
  int d_G = (LG>RG)? LG-RG: RG-LG;
  int d_B = (LB>RB)? LB-RB: RB-LB;
  return d_R + d_G + d_B;
}

void cut(int D, int H, int W) {
  D += Min_depth;
  DEP[IADR(H, W)] = D;
}

void disparity_out(const char *name, int scale) {
  cv::Mat img(SIZE_H, SIZE_W, CV_8U, cv::Scalar(0));
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      int S = DEP[IADR(H, W)];
      int dis = scale*S;
      if (dis > 255) dis = 255;
      img.at<uchar>(H, W) = dis;
    }
  }
  cv::imwrite(name, img);
  img.release();
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 8) {
    printf("Usage: './program_name right.ppm left.ppm max_disparity min_disparity penalty inhibit scale'\n");
    return 1;
  }
  const char *rname = argv[1];
  const char *lname = argv[2];
  rframe = cv::imread(rname); if (rframe.empty()) { printf("cannot read %s\n", rname); return 1; }
  lframe = cv::imread(lname); if (lframe.empty()) { printf("cannot read %s\n", lname); return 1; }
  if ((rframe.cols != lframe.cols)
    ||(rframe.rows != lframe.rows)) {
    printf("Two images's sizes are not equal.\n");
    return 1;
  }
  Max_depth = atoi(argv[3]);
  Min_depth = atoi(argv[4]);
  int penalty = atoi(argv[5]);
  int inhibit = atoi(argv[6]);
  int scale = atoi(argv[7]);
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = rframe.rows;
  SIZE_W = rframe.cols;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  DEP = new int[SIZE_HW];

  graph_cut(penalty, penalty, inhibit, 0);
  std::string str = "disparity"+std::to_string(penalty)+".png";
  disparity_out(str.c_str(), scale);
  printf("%s\n", str.c_str());

  delete [] DEP;
  lframe.release();
  rframe.release();
  return 0;
}
