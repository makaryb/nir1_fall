/********************************************************************/
/***** Stereo Vision using Gaze_line-Depth Model ********************/
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
#include <png++/png.hpp>

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
  int RW;
  int LW;
  if ((W-D < 0) && (W+D > SIZE_W-1)) {
    printf("Error\n");
  }
  else if (W-D < 0) {
    RW = 0;
    LW = 2*W;
  }
  else if (W+D > SIZE_W-1) {
    LW = SIZE_W-1;
    RW = SIZE_W-1-2*(SIZE_W-1-W);
  }
  else {
    RW = W-D;
    LW = W+D;
  }
  call_rgb(rframe, H, RW, &RR, &RG, &RB);
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

void depth_out(const char *name) {
  png::image<png::gray_pixel> img(SIZE_W, SIZE_H);
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      int dep = DEP[IADR(H, W)];
      if (dep > 255) dep = 255;
      img.set_pixel(W, H, dep);
    }
  }
  img.write(name);
}

void disparity_out(const char *name) {
  png::image<png::gray_pixel_16> img(SIZE_W, SIZE_H);
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = SIZE_W-1; W >= 0; W--) {
      for (int D = Max_depth; D >= Min_depth; D--) {
        if (W-D >= 0) {
          if (D <= DEP[IADR(H, W-D)]) {
            int dis = 256*2*D;
            if (dis > 256*256-1) dis = 256*256-1;
            if (W >= 2*Min_depth) img.set_pixel(W, H, dis);
            break;
          }
        }
      }
    }
  }
  img.write(name);
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 7) {
    printf("Usage: './depth right.png left.png max_disparity min_disparity penalty inhibit'\n");
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
  Max_depth = atoi(argv[3])/2;
  Min_depth = atoi(argv[4])/2;
  int penalty = atoi(argv[5]);
  int inhibit = atoi(argv[6]);
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = rframe.rows;
  SIZE_W = rframe.cols;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  DEP = new int[SIZE_HW];

  graph_cut(penalty, penalty, inhibit, inhibit);
  std::string dep_str = "dep_pen_"+std::to_string(penalty)+"_inh_"+std::to_string(inhibit)+".png";
  depth_out(dep_str.c_str());
  printf("%s\n", dep_str.c_str());
  std::string dis_str = "dis_pen_"+std::to_string(penalty)+"_inh_"+std::to_string(inhibit)+".png";
  disparity_out(dis_str.c_str());
  printf("%s\n", dis_str.c_str());

  delete [] DEP;
  lframe.release();
  rframe.release();
  return 0;
}
