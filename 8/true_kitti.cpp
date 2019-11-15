/********************************************************************/
/***** make true disparity file *************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.15 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

#include <stdio.h>
#include <png++/png.hpp>

#define W_SIZE (256*2*(XX-1)+ZZ+1)
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

void dis_to_dis(const char *i_name, const char *o_name, const char *e_name) {
  png::image<png::gray_pixel_16> frame(i_name);
  int XX = frame.get_width();
  int YY = frame.get_height();
  int ZZ = 0;
  int WW = 256*256;
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int dis = frame.get_pixel(x, y);
      if (dis > ZZ) ZZ = dis;
      if (dis > 0) if (dis < WW) WW = dis;
    }
  }
  printf("XX=%d YY=%d max(scaled)disparity=%g(%d) min(scaled)disparite=%g(%d)\n", XX, YY, (double)ZZ/256, ZZ, (double)WW/256, WW);
  position *buf = new position[BUF_NUM];
  for (int i = 0; i < BUF_NUM; i++) buf[i].dis = 0;
  /***** translate from pixel-disparity to gaze_line-depth *****/
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int dis = frame.get_pixel(x, y);
      if (dis > 0) {
        ///////////////////////////////////////////////////////
        int W = 256*2*x - dis + ZZ;
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
  int right_occul = 0;
  int left_occul = 0;
  for (int H = 0; H < H_SIZE; H++) {
    int line_right_occul = 0;
    int line_left_occul = 0;
    for (int W = 0; W < W_SIZE; W++) {
      int S = buf[BUF_ADR(H, W)].dis;
      if (S > 0) {
        int r = 1;
        for ( ; ; ) {
          int RW = W + r;
          if (RW == W_SIZE) break;
          int SS = buf[BUF_ADR(H, RW)].dis;
          if (SS > 0) {
            if (SS < S-r) {
              buf[BUF_ADR(H, RW)].dis = -1;
            //printf("Detect right occulusion = %d\n", S-r-SS);
              line_right_occul++;
            }
          }
          r++;
        }
        int l = 1;
        for ( ; ; ) {
          int LW = W - l;
          if (LW == -1) break;
          int SS = buf[BUF_ADR(H, LW)].dis;
          if (SS > 0) {
            if (SS < S-l) {
              buf[BUF_ADR(H, LW)].dis = -1;
            //printf("Detect left occulusion = %d\n", S-l-SS);
              line_left_occul++;
            }
          }
          l++;
        }
      }
    }
    printf("line %3d detected occulusion: right=%2d left=%2d\n", H, line_right_occul, line_left_occul);
    right_occul += line_right_occul;
    left_occul += line_left_occul;
  }
  /***** make true disparity *****/
  png::image<png::gray_pixel_16> truth(XX, YY);
  png::image<png::rgb_pixel> error(XX, YY);
  for (int H = 0; H < H_SIZE; H++) {
    for (int W = 0; W < W_SIZE; W++) {
      if (buf[BUF_ADR(H, W)].dis > 0) {
        int x = buf[BUF_ADR(H, W)].x;
        int y = buf[BUF_ADR(H, W)].y;
        int dis = buf[BUF_ADR(H, W)].dis;
        if (x >= WW/256) truth.set_pixel(x, y, dis);
      }
      if (buf[BUF_ADR(H, W)].dis == -1) {
        int x = buf[BUF_ADR(H, W)].x;
        int y = buf[BUF_ADR(H, W)].y;
        png::rgb_pixel val;
        val.red   = 255;
        val.green =   0;
        val.blue  =   0;
        error.set_pixel(x, y, val);
      }
    }
  }
  truth.write(o_name);
  error.write(e_name);
  printf("XX=%d YY=%d max(scaled)disparity=%g(%d) min(scaled)disparite=%g(%d)\n", XX, YY, (double)ZZ/256, ZZ, (double)WW/256, WW);
  printf("detected occulusion: right=%d left=%d total=%d\n", right_occul, left_occul, right_occul+left_occul);
}

/********************************************************************/
int main(int argc, const char** argv) {
  if (argc == 4) {
    dis_to_dis(argv[1], argv[2], argv[3]);
    return 0;
  }
  else {
    printf("Usage: './true input_disparity.png output_true_disparity.png output_error.png'\n");
    return 1;
  }
}
