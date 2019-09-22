/********************************************************************/
/***** Stereo Vision using Gaze_line-Depth_number Model *************/
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
int strech;
int Max_depth;
int Min_depth;

#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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

void disparity_out(const char *name, int scale) {
  cv::Mat img(SIZE_H, SIZE_W, CV_8U, cv::Scalar(0));
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      for (int D = Max_depth; D >= Min_depth; D--) {
        if (W+D < SIZE_W) {
          if (D <= DEP[IADR(H, W+D)]) {
            int dis = scale*2*D/strech;
            if (dis > 255) dis = 255;
            img.at<uchar>(H, W) = dis;
            break;
          }
        }
      }
    }
  }
  resize(img, img, cv::Size(), 1.0/strech, 1.0/strech, cv::INTER_AREA);
  cv::imwrite(name, img);
  img.release();
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 9) {
    printf("Usage: './program_name right.ppm left.ppm max_disparity min_disparity penalty inhibit strech scale'\n");
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
  strech = atoi(argv[7]);
  int scale = atoi(argv[8]);
  Max_depth = strech*atoi(argv[3])/2;
  Min_depth = strech*atoi(argv[4])/2;
  int penalty = atoi(argv[5]);
  int inhibit = atoi(argv[6]);
  resize(rframe, rframe, cv::Size(), strech, strech, cv::INTER_CUBIC);
  resize(lframe, lframe, cv::Size(), strech, strech, cv::INTER_CUBIC);
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = rframe.rows;
  SIZE_W = rframe.cols;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  DEP = new int[SIZE_HW];

  graph_cut(penalty, penalty, inhibit, inhibit);
  std::string str = "pen_"+std::to_string(penalty)+"_inh_"+std::to_string(inhibit)+".png";
  disparity_out(str.c_str(), scale);
  printf("%s\n", str.c_str());

  delete [] DEP;
  lframe.release();
  rframe.release();
  return 0;
}
