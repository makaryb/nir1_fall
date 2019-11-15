/********************************************************************/
/***** 3D Stereo Viewer *********************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

int SIZE_H;
int SIZE_W;
int SIZE_HW;
int Max_depth;
int Min_depth;

#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <opencv2/highgui/highgui.hpp>
#include <png++/png.hpp>

cv::Mat rframe;
cv::Mat lframe;

int *DEP;
int *DIS;

#define IADR(H, W) \
 ((H)*SIZE_W+\
  (W))

#define PI (3.141592653589793238462643383279502884197)
#define HALF_BASE (0.05)
#define FOCAL (0.00814)
#define PIXEL (0.00001)
#define LOOK_AT_R (1.0)
#define LOOK_AT_1 (0.0)
#define LOOK_AT_2 (0.0)
#define TARGET_X (-1.0)
#define TARGET_Y (0.0)
#define TARGET_Z (0.0)
#define Right (0)
#define Left (1)
#define Stereo (2)
#define Both (3)
#define DELTA (0.02)

double half_base;
double focal;
double pixel;
double look_at_r;
double look_at_1;
double look_at_2;
double target_x;
double target_y;
double target_z;
int depth_num;
int disp_mode;
int figure;
int dis_mode;
int l_cursol_x;
int r_cursol_x;
int cursol_y;

void call_rgb(cv::Mat &frame, int y, int x, int *r, int *g, int *b) {
  if (x < 0) x = 0;
  if (x > SIZE_W-1) x = SIZE_W-1;
  cv::Vec3b bgr = frame.at<cv::Vec3b>(y,x);
  *b = bgr[0]; *g = bgr[1]; *r = bgr[2];
}

void display_hypo(void) {
  glPointSize(2);
  glBegin(GL_POINTS);
  for (int h = -SIZE_H/2; h < SIZE_H/2; h++) {
    int hh = h + SIZE_H/2;
    for (int w = -SIZE_W/2; w < SIZE_W/2; w++) {
      int ww = w + SIZE_W/2;
      int d;
      double r, l;
      if (disp_mode == Stereo) {
        if (dis_mode == 0) {
          d = DEP[IADR(hh, ww)];
          if (d == 0) continue;
          r = w-d;
          l = w+d;
        }
        else if (dis_mode == 1) {
          d = DIS[IADR(hh, ww)];
          if (d == 0) continue;
          r = w-(double)d/256;
          l = w;
          d /= 256;
        }
        else {
        }
      }
      else {
        d = depth_num;
        r = w-d;
        l = w+d;
      }
      double x = -2*focal*half_base/((l-r)*pixel);
      double y =  half_base*(l+r)/(l-r);
      double z = -2*h*half_base/(l-r);
      int RR, RG, RB;
      int LR, LG, LB;
      double R, G, B;
      if (disp_mode == Stereo) {
        call_rgb(rframe, hh, ww-d, &RR, &RG, &RB);
        if (dis_mode == 0) call_rgb(lframe, hh, ww+d, &LR, &LG, &LB);
        else               call_rgb(lframe, hh, ww, &LR, &LG, &LB);
        R = ((double)(RR+LR))/512;
        G = ((double)(RG+LG))/512;
        B = ((double)(RB+LB))/512;
      }
      else if (disp_mode == Right) {
        call_rgb(rframe, hh, ww-d, &RR, &RG, &RB);
        R = ((double)RR)/256;
        G = ((double)RG)/256;
        B = ((double)RB)/256;
      }
      else if (disp_mode == Left) {
        call_rgb(lframe, hh, ww+d, &LR, &LG, &LB);
        R = ((double)LR)/256;
        G = ((double)LG)/256;
        B = ((double)LB)/256;
      }
      else if (disp_mode == Both) {
        call_rgb(rframe, hh, ww-d, &RR, &RG, &RB);
        call_rgb(lframe, hh, ww+d, &LR, &LG, &LB);
        R = ((double)(RR+LR))/512;
        G = ((double)(RG+LG))/512;
        B = ((double)(RB+LB))/512;
      }
      else { }
      glColor3d(R, G, B);
      glVertex3d(x, y, z);
    }
  }
  glEnd();
}

void right_cursol_vertical(void) {
  glColor3d(0.0, 1.0, 1.0);
  int r = r_cursol_x-SIZE_W/2;
  int l = r_cursol_x-SIZE_W/2+2*depth_num;
  double x = -2*focal*half_base/((l-r)*pixel);
  double y =  half_base*(l+r)/(l-r);
  double z1 = -2*(-SIZE_H/2)*half_base/(l-r);
  double z2 = -2*(SIZE_H/2-1)*half_base/(l-r);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(x+DELTA, y, z1);
  glVertex3d(x+DELTA, y, z2);
  glEnd();
}

void right_cursol_horizontal(void) {
  glColor3d(0.0, 1.0, 1.0);
  int r1 = -SIZE_W/2;
  int l1 = -SIZE_W/2+2*depth_num;
  int r2 = SIZE_W/2-1;
  int l2 = SIZE_W/2-1+2*depth_num;
  double x1 = -2*focal*half_base/((l1-r1)*pixel);
  double y1 =  half_base*(l1+r1)/(l1-r1);
  double z1 = -2*(cursol_y-SIZE_H/2)*half_base/(l1-r1);
  double x2 = -2*focal*half_base/((l2-r2)*pixel);
  double y2 =  half_base*(l2+r2)/(l2-r2);
  double z2 = -2*(cursol_y-SIZE_H/2)*half_base/(l2-r2);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(x1+DELTA, y1, z1);
  glVertex3d(x2+DELTA, y2, z2);
  glEnd();
}

void to_right_cursol_center(void) {
  glColor3d(0.0, 1.0, 1.0);
  int r = r_cursol_x-SIZE_W/2;
  int l = r_cursol_x-SIZE_W/2+2*depth_num;
  double x = -2*focal*half_base/((l-r)*pixel);
  double y =  half_base*(l+r)/(l-r);
  double z = -2*(cursol_y-SIZE_H/2)*half_base/(l-r);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(0.0,  half_base, 0.0);
  glVertex3d(x+DELTA, y, z);
  glEnd();
  glPointSize(4);
  glBegin(GL_POINTS);
  glVertex3d(0.0,  half_base, 0.0);
  glEnd();
}

void right_picture(void) {
  glPointSize(1);
  glBegin(GL_POINTS);
  for (int h = -SIZE_H/2; h < SIZE_H/2; h++) {
    int hh = h + SIZE_H/2;
    for (int w = -SIZE_W/2; w < SIZE_W/2; w++) {
      int ww = w + SIZE_W/2;
      int R, G, B;
      call_rgb(rframe, hh, ww, &R, &G, &B);
      glColor3d((double)R/255, (double)G/255, (double)B/255);
      glVertex3d(-focal, half_base+w*pixel, -h*pixel);
    }
  }
  glEnd();
}

void left_cursol_vertical(void) {
  glColor3d(1.0, 1.0, 0.0);
  int r = l_cursol_x-SIZE_W/2-2*depth_num;
  int l = l_cursol_x-SIZE_W/2;
  double x = -2*focal*half_base/((l-r)*pixel);
  double y =  half_base*(l+r)/(l-r);
  double z1 = -2*(-SIZE_H/2)*half_base/(l-r);
  double z2 = -2*(SIZE_H/2-1)*half_base/(l-r);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(x+DELTA, y, z1);
  glVertex3d(x+DELTA, y, z2);
  glEnd();
}

void left_cursol_horizontal(void) {
  glColor3d(1.0, 1.0, 0.0);
  int r1 = -SIZE_W/2-2*depth_num;
  int l1 = -SIZE_W/2;
  int r2 = SIZE_W/2-1-2*depth_num;
  int l2 = SIZE_W/2-1;
  double x1 = -2*focal*half_base/((l1-r1)*pixel);
  double y1 =  half_base*(l1+r1)/(l1-r1);
  double z1 = -2*(cursol_y-SIZE_H/2)*half_base/(l1-r1);
  double x2 = -2*focal*half_base/((l2-r2)*pixel);
  double y2 =  half_base*(l2+r2)/(l2-r2);
  double z2 = -2*(cursol_y-SIZE_H/2)*half_base/(l2-r2);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(x1+DELTA, y1, z1);
  glVertex3d(x2+DELTA, y2, z2);
  glEnd();
}

void to_left_cursol_center(void) {
  glColor3d(1.0, 1.0, 0.0);
  int r = l_cursol_x-SIZE_W/2-2*depth_num;
  int l = l_cursol_x-SIZE_W/2;
  double x = -2*focal*half_base/((l-r)*pixel);
  double y =  half_base*(l+r)/(l-r);
  double z = -2*(cursol_y-SIZE_H/2)*half_base/(l-r);
  glPointSize(2);
  glBegin(GL_LINES);
  glVertex3d(0.0, -half_base, 0.0);
  glVertex3d(x+DELTA, y, z);
  glEnd();
  glPointSize(4);
  glBegin(GL_POINTS);
  glVertex3d(0.0, -half_base, 0.0);
  glEnd();
}

void left_picture(void) {
  glPointSize(1);
  glBegin(GL_POINTS);
  for (int h = -SIZE_H/2; h < SIZE_H/2; h++) {
    int hh = h + SIZE_H/2;
    for (int w = -SIZE_W/2; w < SIZE_W/2; w++) {
      int ww = w + SIZE_W/2;
      int R, G, B;
      call_rgb(lframe, hh, ww, &R, &G, &B);
      glColor3d((double)R/255, (double)G/255, (double)B/255);
      glVertex3d(-focal, -half_base+w*pixel, -h*pixel);
    }
  }
  glEnd();
}

void display_all(void) {
  glEnable(GL_DEPTH_TEST);
  if (figure) {
    if (disp_mode != Left) {
      right_cursol_vertical();
      right_cursol_horizontal();
      to_right_cursol_center();
      right_picture();
    }
    if (disp_mode != Right) {
      left_cursol_vertical();
      left_cursol_horizontal();
      to_left_cursol_center();
      left_picture();
    }
  }
  display_hypo();
  glDisable(GL_DEPTH_TEST);
}

void display(void) {
  double p = look_at_r * cos(look_at_1);
  double z = look_at_r * sin(look_at_1) + target_z;
  double x =         p * cos(look_at_2) + target_x;
  double y =         p * sin(look_at_2) + target_y;
  glLoadIdentity();
  if (p >= 0.0) gluLookAt(x, y, z, target_x, target_y, target_z, 0, 0,  1);
  else          gluLookAt(x, y, z, target_x, target_y, target_z, 0, 0, -1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  display_all();
  glutSwapBuffers();
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30, (double)w / h, 0.01, 100);
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') {
    glutLeaveMainLoop();
    return;
  }
  /*------------------------------*/
  if (key == '?') {
    printf("q: quit\n");
    printf("?: help\n");
    printf("6: video(result.avi) out\n");
    printf("e: toggle figure mode\n");
    printf("w: toggle dis_mode\n");
    printf("m: left picture\n");
    printf("<: stereo picture\n");
    printf(".: right picture\n");
    printf(",: both picture\n");
    printf("s: depth(disparity) to front\n");
    printf("t: depth(disparity) to back\n");
    printf("y: left cursol left\n");
    printf("u: left cursol down\n");
    printf("i: left cursol up\n");
    printf("o: left cursol right\n");
    printf("7: right cursol left\n");
    printf("8: right cursol down\n");
    printf("9: right cursol up\n");
    printf("0: right cursol right\n");
    printf("h: eye move left angle\n");
    printf("j: eye move down angle\n");
    printf("k: eye move up angle\n");
    printf("l: eye move right angle\n");
    printf("f: eye move far\n");
    printf("n: eye move near\n");
    printf("H: eye move left\n");
    printf("J: eye move down\n");
    printf("K: eye move up\n");
    printf("L: eye move right\n");
    printf("F: eye move forward\n");
    printf("N: eye move back\n");
    printf("C: clear angle and position\n");
    printf("!: decrease base line\n");
    printf("@: increase base line\n");
    printf("#: decrease focal length\n");
    printf("$: increase focal length\n");
    printf("%%: decrease pixel size\n");
    printf("^: increase pixel size\n");
    return;
  }
  /*------------------------------*/
  if (key == '6') {
    figure = 0;
    look_at_r = LOOK_AT_R;
    look_at_1 = LOOK_AT_1;
    look_at_2 = LOOK_AT_2;
    target_x = TARGET_X;
    target_y = TARGET_Y;
    target_z = TARGET_Z;
    #define PADR(H, W, C) \
     ((H)*4*SIZE_W+\
      (W)*4+\
      (C))
    int time_a =        100;
    int time_b = time_a+ 50;
    int time_c = time_b+ 50;
    int time_d = time_c+100;
    int time_e = time_d+300;
    uchar *pix = new uchar[SIZE_H*SIZE_W*4];
    cv::VideoWriter writer("result.avi", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(SIZE_W, SIZE_H));
    cv::Mat frame(cv::Size(SIZE_W, SIZE_H), CV_8UC3);
    for (int i = 0; i < time_e; i++) {
      display();
      glReadPixels(0, 0, SIZE_W, SIZE_H, GL_RGBA, GL_UNSIGNED_BYTE, pix);
      for (int y = 0; y < SIZE_H; y++) {
        for (int x = 0; x < SIZE_W; x++) {
          frame.at<cv::Vec3b>(y,x)[0] = pix[PADR(SIZE_H-1-y, x, 2)];
          frame.at<cv::Vec3b>(y,x)[1] = pix[PADR(SIZE_H-1-y, x, 1)];
          frame.at<cv::Vec3b>(y,x)[2] = pix[PADR(SIZE_H-1-y, x, 0)];
        }
      }
      writer << frame;
      if      (i < time_a) {                      target_x -= 0.01; }
      else if (i < time_b) { look_at_2 -= PI/180; target_x -= 0.01; }
      else if (i < time_c) { look_at_2 += PI/180; target_x -= 0.01; }
      else if (i < time_d) {                      target_x -= 0.01; }
      else                 {                      target_x += 0.01; }
    }
    frame.release();
    writer.release();
    delete [] pix;
    return;
  }
  /*------------------------------*/
  if (key == 'e') {
    if      (figure == 0) figure = 1;
    else if (figure == 1) figure = 0;
    else ;
    printf("figure=%d\n", figure);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'w') {
    if      (dis_mode == 0) dis_mode = 1;
    else if (dis_mode == 1) dis_mode = 0;
    else ;
    printf("dis_mode=%d\n", dis_mode);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'm') {
    disp_mode = Left;
    glutPostRedisplay();
    return;
  }
  if (key == '<') {
    disp_mode = Stereo;
    glutPostRedisplay();
    return;
  }
  if (key == '.') {
    disp_mode = Right;
    glutPostRedisplay();
    return;
  }
  if (key == ',') {
    disp_mode = Both;
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 's') {
    depth_num++;
    if (depth_num > Max_depth) depth_num = Max_depth;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num);
    glutPostRedisplay();
    return;
  }
  if (key == 't') {
    depth_num--;
    if (depth_num < Min_depth) depth_num = Min_depth;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'y') {
    l_cursol_x--;
    if (l_cursol_x < 0) l_cursol_x = 0;
    printf("disparity(%d, %d)=%g\n", l_cursol_x, cursol_y, (double)DIS[IADR(cursol_y, l_cursol_x)]/256);
    glutPostRedisplay();
    return;
  }
  if (key == 'u') {
    cursol_y++;
    if (cursol_y > SIZE_H-1) cursol_y = SIZE_H-1;
    printf("disparity(%d, %d)=%g\n", l_cursol_x, cursol_y, (double)DIS[IADR(cursol_y, l_cursol_x)]/256);
    glutPostRedisplay();
    return;
  }
  if (key == 'i') {
    cursol_y--;
    if (cursol_y < 0) cursol_y = 0;
    printf("disparity(%d, %d)=%g\n", l_cursol_x, cursol_y, (double)DIS[IADR(cursol_y, l_cursol_x)]/256);
    glutPostRedisplay();
    return;
  }
  if (key == 'o') {
    l_cursol_x++;
    if (l_cursol_x > SIZE_W-1) l_cursol_x = SIZE_W-1;
    printf("disparity(%d, %d)=%g\n", l_cursol_x, cursol_y, (double)DIS[IADR(cursol_y, l_cursol_x)]/256);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == '7') {
    r_cursol_x--;
    if (r_cursol_x < 0) r_cursol_x = 0;
    if (r_cursol_x+depth_num < SIZE_W) printf("depth(%d, %d)=%d\n", r_cursol_x, cursol_y, DEP[IADR(cursol_y, r_cursol_x+depth_num)]);
    glutPostRedisplay();
    return;
  }
  if (key == '8') {
    cursol_y++;
    if (cursol_y > SIZE_H-1) cursol_y = SIZE_H-1;
    if (r_cursol_x+depth_num < SIZE_W) printf("depth(%d, %d)=%d\n", r_cursol_x, cursol_y, DEP[IADR(cursol_y, r_cursol_x+depth_num)]);
    glutPostRedisplay();
    return;
  }
  if (key == '9') {
    cursol_y--;
    if (cursol_y < 0) cursol_y = 0;
    if (r_cursol_x+depth_num < SIZE_W) printf("depth(%d, %d)=%d\n", r_cursol_x, cursol_y, DEP[IADR(cursol_y, r_cursol_x+depth_num)]);
    glutPostRedisplay();
    return;
  }
  if (key == '0') {
    r_cursol_x++;
    if (r_cursol_x > SIZE_W-1) r_cursol_x = SIZE_W-1;
    if (r_cursol_x+depth_num < SIZE_W) printf("depth(%d, %d)=%d\n", r_cursol_x, cursol_y, DEP[IADR(cursol_y, r_cursol_x+depth_num)]);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'h') {
    look_at_2 -= PI / 180;
    glutPostRedisplay();
    return;
  }
  if (key == 'j') {
    look_at_1 -= PI / 180;
    glutPostRedisplay();
    return;
  }
  if (key == 'k') {
    look_at_1 += PI / 180;
    glutPostRedisplay();
    return;
  }
  if (key == 'l') {
    look_at_2 += PI / 180;
    glutPostRedisplay();
    return;
  }
  if (key == 'f') {
    look_at_r /= 0.9;
    glutPostRedisplay();
    return;
  }
  if (key == 'n') {
    look_at_r *= 0.9;
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'H') {
    target_y -= 0.01;
    glutPostRedisplay();
    return;
  }
  if (key == 'J') {
    target_z -= 0.01;
    glutPostRedisplay();
    return;
  }
  if (key == 'K') {
    target_z += 0.01;
    glutPostRedisplay();
    return;
  }
  if (key == 'L') {
    target_y += 0.01;
    glutPostRedisplay();
    return;
  }
  if (key == 'F') {
    target_x += 0.01;
    glutPostRedisplay();
    return;
  }
  if (key == 'N') {
    target_x -= 0.01;
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 'C') {
    look_at_r = LOOK_AT_R;
    look_at_1 = LOOK_AT_1;
    look_at_2 = LOOK_AT_2;
    target_x = TARGET_X;
    target_y = TARGET_Y;
    target_z = TARGET_Z;
    printf("clear angle and position\n");
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == '!') {
    half_base /= 1.1;
    printf("base line=%g\n", 2*half_base);
    glutPostRedisplay();
    return;
  }
  if (key == '@') {
    half_base *= 1.1;
    printf("base line=%g\n", 2*half_base);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == '#') {
    focal /= 1.01;
    printf("focal length=%g\n", focal);
    glutPostRedisplay();
    return;
  }
  if (key == '$') {
    focal *= 1.01;
    printf("focal length=%g\n", focal);
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == '%') {
    pixel /= 1.1;
    printf("pixel size=%g\n", pixel);
    focal /= 1.1;
    glutPostRedisplay();
    return;
  }
  if (key == '^') {
    pixel *= 1.1;
    printf("pixel size=%g\n", pixel);
    focal *= 1.1;
    glutPostRedisplay();
    return;
  }
}

int load_DEP(const char *name) {
  png::image<png::gray_pixel> frame(name);
  if ((frame.get_width() != SIZE_W)||(frame.get_height() != SIZE_H)) {
    printf("Depth's size is not equal to Left/Right.\n");
    return 1;
  }
  int MX = 0;
  int MN = 256;
  for (int h = 0; h < SIZE_H; h++) {
    for (int w = 0; w < SIZE_W; w++) {
      int dep = frame.get_pixel(w, h);
      DEP[IADR(h, w)] = dep;
      if (dep > MX) MX = dep;
      if (dep > 0) if (dep < MN) MN = dep;
    }
  }
  printf("max_depth=%d min_depth=%d\n", MX, MN);
  return 0;
}

int load_DIS(const char *name) {
  png::image<png::gray_pixel_16> frame(name);
  if ((frame.get_width() != SIZE_W)||(frame.get_height() != SIZE_H)) {
    printf("Disparity's size is not equal to Left/Right.\n");
    return 1;
  }
  int MX = 0;
  int MN = 256*256;
  for (int h = 0; h < SIZE_H; h++) {
    for (int w = 0; w < SIZE_W; w++) {
      int dis = frame.get_pixel(w, h);
      DIS[IADR(h, w)] = dis;
      if (dis > MX) MX = dis;
      if (dis > 0) if (dis < MN) MN = dis;
    }
  }
  printf("max_disparity=%g min_disparite=%g\n", (double)MX/256, (double)MN/256);
  return 0;
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 7) {
    printf("Usage: './3d right.png left.png max_disparity min_disparity depth.png disparity.png'\n");
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
  SIZE_H = rframe.rows;
  SIZE_W = rframe.cols;
  SIZE_HW = SIZE_H*SIZE_W;
  DEP = new int[SIZE_HW];
  DIS = new int[SIZE_HW];
  if (load_DEP(argv[5])) return 1;
  if (load_DIS(argv[6])) return 1;

  half_base = HALF_BASE;
  focal = FOCAL;
  pixel = PIXEL;
  look_at_r = LOOK_AT_R;
  look_at_1 = LOOK_AT_1;
  look_at_2 = LOOK_AT_2;
  target_x = TARGET_X;
  target_y = TARGET_Y;
  target_z = TARGET_Z;
  depth_num = Min_depth;
  disp_mode = Right;
  figure = 0;
  dis_mode = 0;
  l_cursol_x = SIZE_W/2;
  r_cursol_x = SIZE_W/2;
  cursol_y = SIZE_H/2;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (SIZE_W, SIZE_H);
  glutInitWindowPosition (390, 44);
  glutCreateWindow("Stereo Vision by K. OGURI 20180214");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(100);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();

  delete [] DIS;
  delete [] DEP;
  lframe.release();
  rframe.release();
  return 0;
}
