/********************************************************************/
/***** compare disparities ******************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.15 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

#include <stdio.h>
#include <png++/png.hpp>
#define OFFSET (128)

void compare_disparity(const char *t_name, const char *p_name, const char *e_name) {
  png::image<png::gray_pixel_16> t_frame(t_name);
  png::image<png::gray_pixel>    p_frame(p_name);
  if ((t_frame.get_width()  != p_frame.get_width())
    ||(t_frame.get_height() != p_frame.get_height())) {
    printf("Two images's sizes are not equal.\n");
    return;
  }
  int XX = t_frame.get_width();
  int YY = t_frame.get_height();
  int count = 0;
  int P_MAX = 0;
  int M_MAX = 0;
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      int T = t_frame.get_pixel(x, y);
      if (T > 0) {
        count++;
        T = (T+OFFSET)/256;
        int P = p_frame.get_pixel(x, y);
        int D = P - T;
        if (D > 0) { if ( D > P_MAX) P_MAX =  D; }
        if (D < 0) { if (-D > M_MAX) M_MAX = -D; }
      }
    }
  }
  int MAX = (P_MAX >= M_MAX)? P_MAX: M_MAX;
  int p_eval[MAX+1] = {};
  int m_eval[MAX+1] = {};
  png::image<png::rgb_pixel> error(XX, YY);
  for (int y = 0; y < YY; y++) {
    for (int x = 0; x < XX; x++) {
      png::rgb_pixel val;
      int T = t_frame.get_pixel(x, y);
      if (T > 0) {
        T = (T+OFFSET)/256;
        int P = p_frame.get_pixel(x, y);
        int D = P - T;
        if (D > 0) { p_eval[ D]++; val.red = 255; val.green = 0; val.blue = 0; error.set_pixel(x, y, val); }
        if (D < 0) { m_eval[-D]++; val.red = 0; val.green = 255; val.blue = 0; error.set_pixel(x, y, val); }
      }
    }
  }
  error.write(e_name);
//for (int i = 1; i <= MAX; i++) {
//  if ((p_eval[i]!=0)||(m_eval[i]!=0)) {
//    printf("p_eval[%d]=%d(%d%%)\tm_eval[%d]=%d(%d%%)\n",
//    i, p_eval[i], 100*p_eval[i]/count,
//    i, m_eval[i], 100*m_eval[i]/count);
//  }
//}
  int p_total = 0;
  int m_total = 0;
  for (int i = 1; i <= MAX; i++) {
    p_total += i*p_eval[i];
    m_total += i*m_eval[i];
  }
  printf("p_total=%d m_total=%d total=%d", p_total, m_total, p_total+m_total);
  printf(" average=%gpx\n", (double)(p_total+m_total)/count);
}

/********************************************************************/
int main(int argc, const char** argv) {
  if (argc == 4) {
    compare_disparity(argv[1], argv[2], argv[3]);
    return 0;
  }
  else {
    printf("Usage: './cmp true_16bit_disparity.png 8bit_disparity.png output_error.png'\n");
    return 1;
  }
}
