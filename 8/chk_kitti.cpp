/********************************************************************/
/***** check disparity range ****************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.15 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

#include <stdio.h>
#include <png++/png.hpp>

void check(const char *name, int &max, int &min) {
  png::image<png::gray_pixel_16> frame(name);
  int XX = frame.get_width();
  int YY = frame.get_height();
  int MX = 0;
  int MN = 256*256;
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int dis = frame.get_pixel(x, y);
      if (dis > MX) MX = dis;
      if (dis > 0) if (dis < MN) MN = dis;
    }
  }
//printf("XX=%d YY=%d max_disparity=%g min_disparite=%g\n", XX, YY, (double)MX/256, (double)MN/256);
  max = MX;
  min = MN;
}

/********************************************************************/
int main(int argc, char** argv) {
  int p_max;
  int p_min;
  int MAX = 0;
  int MIN = 256*256;
  for (int i = 0; i < 194; i++) {
    char aaa[256];
    sprintf(aaa, "%06d_10", i);
    std::string prefix = aaa;
    std::string name = "/home/oguri/KITTI/data_2012/training/disp_occ/"+prefix+".png";
  //printf("%s ", name.c_str());
    int max, min;
    check(name.c_str(), max, min);
    if (max > MAX) { MAX = max; p_max = i; }
    if (min < MIN) { MIN = min; p_min = i; }
  }
  printf("OCC: Max_disparity=%g at %d\nOCC: Min_disparity=%g at %d\n", (double)MAX/256, p_max, (double)MIN/256, p_min);
  int OCC[MAX+1] = {};
  for (int i = 0; i < 194; i++) {
    char aaa[256];
    sprintf(aaa, "%06d_10", i);
    std::string prefix = aaa;
    std::string name = "/home/oguri/KITTI/data_2012/training/disp_occ/"+prefix+".png";
    int max, min;
    check(name.c_str(), max, min);
    OCC[max]++;
  }
  int total = 0;
  for (int i = 0; i <= MAX; i++) {
    total += OCC[i];
    if (total >= 194*80/100) {
      printf("disparity=%g at 80%%\n\n", (double)i/256);
      break;
    }
  }

  MAX = 0;
  MIN = 256*256;
  for (int i = 0; i < 194; i++) {
    char aaa[256];
    sprintf(aaa, "%06d_10", i);
    std::string prefix = aaa;
    std::string name = "/home/oguri/KITTI/data_2012/training/disp_noc/"+prefix+".png";
  //printf("%s ", name.c_str());
    int max, min;
    check(name.c_str(), max, min);
    if (max > MAX) { MAX = max; p_max = i; }
    if (min < MIN) { MIN = min; p_min = i; }
  }
  printf("NOC: Max_disparity=%g at %d\nNOC: Min_disparity=%g at %d\n", (double)MAX/256, p_max, (double)MIN/256, p_min);
  int NOC[MAX+1] = {};
  for (int i = 0; i < 194; i++) {
    char aaa[256];
    sprintf(aaa, "%06d_10", i);
    std::string prefix = aaa;
    std::string name = "/home/oguri/KITTI/data_2012/training/disp_noc/"+prefix+".png";
    int max, min;
    check(name.c_str(), max, min);
    NOC[max]++;
  }
  total = 0;
  for (int i = 0; i <= MAX; i++) {
    total += NOC[i];
    if (total >= 194*80/100) {
      printf("disparity=%g at 80%%\n", (double)i/256);
      break;
    }
  }

  return 0;
}
