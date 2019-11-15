/********************************************************************/
/***** Stereo Rectifier using Gaze_line-Depth Model *****************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.04.25 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

extern int graph_cut(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b);
extern int SIZE_S;
extern int SIZE_H;
extern int SIZE_W;
extern int SIZE_HW;
extern int SIZE_SHW;

#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int Max_disp;
int Min_disp;
int penalty;
int inhibit;
int divisor;
int argvflg;
int XX;
int YY;
int *DEP;
int Max_depth;
int Min_depth;
int depth_num;
int DDDD;

#define PI (3.141592653589793238462643383279502884197)
#define HALF_BASE (0.05)
#define FOCAL (0.0814)
#define PIXEL (0.0001)
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

double half_base;
double focal;
double pixel;
double look_at_r;
double look_at_1;
double look_at_2;
double target_x;
double target_y;
double target_z;
int disp_mode;

cv::Mat rframe;
cv::Mat lframe;
cv::Mat rrframe;
cv::Mat llframe;
cv::Mat M;
cv::Mat D;
cv::Size S;

#define IADR(H, W) \
 ((H)*XX+\
  (W))

#define PADR(Y, X, C) \
 ((Y)*4*XX+\
  (X)*4+\
  (C))

void call_rgb(cv::Mat &frame, int y, int x, int *r, int *g, int *b) {
  if (x < 0) x = 0;
  if (x > XX-1) x = XX-1;
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
  call_rgb(rrframe, DDDD*H, DDDD*RW, &RR, &RG, &RB);
  call_rgb(llframe, DDDD*H, DDDD*LW, &LR, &LG, &LB);
  int d_R = (LR>RR)? LR-RR: RR-LR;
  int d_G = (LG>RG)? LG-RG: RG-LG;
  int d_B = (LB>RB)? LB-RB: RB-LB;
  return d_R + d_G + d_B;
}

void cut(int D, int H, int W) {
  D += Min_depth;
  DEP[IADR(DDDD*H, DDDD*W)] = DDDD*D;
}

void display_all(void) {
  glEnable(GL_DEPTH_TEST);
  glPointSize(2);
  glBegin(GL_POINTS);
  for (int h = -YY/2; h < YY/2; h++) {
    int hh = h + YY/2;
    for (int w = -XX/2; w < XX/2; w++) {
      int ww = w + XX/2;
      int d = (disp_mode == Stereo)? DEP[IADR(hh, ww)]: depth_num;
      int r = w-d;
      int l = w+d;
      double x = -2*focal*half_base/((l-r+1)*pixel);
      double y =  half_base*(l+r)/(l-r+1);
      double z = -2*h*half_base/(l-r+1);
      int RR, RG, RB;
      int LR, LG, LB;
      double R, G, B;
      if (disp_mode == Stereo) {
        call_rgb(rrframe, hh, ww-d, &RR, &RG, &RB);
        call_rgb(llframe, hh, ww+d, &LR, &LG, &LB);
        R = ((double)(RR+LR))/512;
        G = ((double)(RG+LG))/512;
        B = ((double)(RB+LB))/512;
      }
      else if (disp_mode == Right) {
        call_rgb(rrframe, hh, ww-d, &RR, &RG, &RB);
        R = ((double)RR)/256;
        G = ((double)RG)/256;
        B = ((double)RB)/256;
      }
      else if (disp_mode == Left) {
        call_rgb(llframe, hh, ww+d, &LR, &LG, &LB);
        R = ((double)LR)/256;
        G = ((double)LG)/256;
        B = ((double)LB)/256;
      }
      else if (disp_mode == Both) {
        call_rgb(rrframe, hh, ww-d, &RR, &RG, &RB);
        call_rgb(llframe, hh, ww+d, &LR, &LG, &LB);
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

void rotate(int l0, int l1, int l2, int r0, int r1, int r2) {
  cv::Mat lV = cv::Mat::zeros(1, 3, CV_64F);
  cv::Mat rV = cv::Mat::zeros(1, 3, CV_64F);
  lV.at<double>(0, 0) = 0.00001*l0;
  lV.at<double>(0, 1) = 0.00001*l1;
  lV.at<double>(0, 2) = 0.00001*l2;
  rV.at<double>(0, 0) = 0.00001*r0;
  rV.at<double>(0, 1) = 0.00001*r1;
  rV.at<double>(0, 2) = 0.00001*r2;
  cv::Mat lR;
  cv::Mat rR;
  Rodrigues(lV, lR);
  Rodrigues(rV, rR);
  cv::Mat lmap1;
  cv::Mat lmap2;
  cv::Mat rmap1;
  cv::Mat rmap2;
  initUndistortRectifyMap(M, D, lR, M, S, CV_16SC2, lmap1, lmap2);
  initUndistortRectifyMap(M, D, rR, M, S, CV_16SC2, rmap1, rmap2);
  remap(lframe, llframe, lmap1, lmap2, cv::INTER_LINEAR);
  remap(rframe, rrframe, rmap1, rmap2, cv::INTER_LINEAR);
}

void cbout(const char *fname, int m,
           int l0i, int l1i, int l2i, int r0i, int r1i, int r2i,
           int l0s, int l1s, int l2s, int r0s, int r1s, int r2s) {
  FILE *fp = fopen(fname, "w");
  fprintf(fp, "# l0i=%d l1i=%d l2i=%d r0i=%d r1i=%d r2i=%d\n", l0i, l1i, l2i, r0i, r1i, r2i);
  for (int l0d = -l0s; l0d <= l0s; l0d++) {
  for (int l1d = -l1s; l1d <= l1s; l1d++) {
  for (int l2d = -l2s; l2d <= l2s; l2d++) {
  for (int r0d = -r0s; r0d <= r0s; r0d++) {
  for (int r1d = -r1s; r1d <= r1s; r1d++) {
  for (int r2d = -r2s; r2d <= r2s; r2d++) {
    rotate(l0i+m*l0d, l1i+m*l1d, l2i+m*l2d, r0i+m*r0d, r1i+m*r1d, r2i+m*r2d);
    int val = graph_cut(penalty, penalty, inhibit, inhibit);
    fprintf(fp, "%d %d %d %d %d %d %d\n", l0i+m*l0d, l1i+m*l1d, l2i+m*l2d, r0i+m*r0d, r1i+m*r1d, r2i+m*r2d, val);
  }
  }
  }
  }
  }
  }
  fclose(fp);
  printf("%s done\n", fname);
}

void disp_and_write(int flg, cv::VideoWriter &writer) {
  if (argvflg) {
    uchar *pix = new uchar[YY*XX*4];
    cv::Mat frame(cv::Size(XX, YY), CV_8UC3);
    disp_mode = flg;
    display();
    glReadPixels(0, 0, XX, YY, GL_RGBA, GL_UNSIGNED_BYTE, pix);
    for (int y = 0; y < YY; y++) {
      for (int x = 0; x < XX; x++) {
        frame.at<cv::Vec3b>(y,x)[0] = pix[PADR(YY-1-y, x, 2)];
        frame.at<cv::Vec3b>(y,x)[1] = pix[PADR(YY-1-y, x, 1)];
        frame.at<cv::Vec3b>(y,x)[2] = pix[PADR(YY-1-y, x, 0)];
      }
    }
    writer << frame;
    frame.release();
    delete [] pix;
  }
  else {
    disp_mode = flg;
    display();
  }
}

void calib(void) {
  cv::VideoWriter writer;
  if (argvflg) writer.open("result.avi", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(XX, YY));
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  DDDD = divisor;
  Max_depth = Max_disp/(2*DDDD);
  Min_depth = Min_disp/(2*DDDD);
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = YY/DDDD;
  SIZE_W = XX/DDDD;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  /*-----------------------*/
  int l0 = 0;
  int l1 = 0;
  int l2 = 0;
  int r0 = 0;
  int r1 = 0;
  int r2 = 0;
  /*-----------------------*/
  rotate(l0, l1, l2, r0, r1, r2);
  disp_and_write(Both, writer);
  int ini = graph_cut(penalty, penalty, inhibit, inhibit);
  int old = ini;
  for (int mul = 512; mul >= 8; mul /= 2) {
    printf("----mul=%2d----\n", mul);
    int flg = 0;
    int step = 0;
    for ( ; ; ) {
           if (step ==  0) l0+=mul;
      else if (step ==  1) r0-=mul;
      else if (step ==  2) l0-=mul;
      else if (step ==  3) r0+=mul;
      else if (step ==  4) l2+=mul;
      else if (step ==  5) r2-=mul;
      else if (step ==  6) l2-=mul;
      else if (step ==  7) r2+=mul;
      else if (step ==  8) l1+=mul;
      else if (step ==  9) r1-=mul;
      else if (step == 10) l1-=mul;
      else if (step == 11) r1+=mul;
      else {}
      rotate(l0, l1, l2, r0, r1, r2);
      disp_and_write(Both, writer);
      int now = graph_cut(penalty, penalty, inhibit, inhibit);
      if (now >= old) {
             if (step ==  0) l0-=mul;
        else if (step ==  1) r0+=mul;
        else if (step ==  2) l0+=mul;
        else if (step ==  3) r0-=mul;
        else if (step ==  4) l2-=mul;
        else if (step ==  5) r2+=mul;
        else if (step ==  6) l2+=mul;
        else if (step ==  7) r2-=mul;
        else if (step ==  8) l1-=mul;
        else if (step ==  9) r1+=mul;
        else if (step == 10) l1+=mul;
        else if (step == 11) r1-=mul;
        else {}
      }
      else {
        flg = 1;
        old = now;
             if (step ==  0) printf("    l0+\n");
        else if (step ==  1) printf("    r0-\n");
        else if (step ==  2) printf("    l0-\n");
        else if (step ==  3) printf("    r0+\n");
        else if (step ==  4) printf("    l2+\n");
        else if (step ==  5) printf("    r2-\n");
        else if (step ==  6) printf("    l2-\n");
        else if (step ==  7) printf("    r2+\n");
        else if (step ==  8) printf("    l1+\n");
        else if (step ==  9) printf("    r1-\n");
        else if (step == 10) printf("    l1-\n");
        else if (step == 11) printf("    r1+\n");
        else ;
      }
      step++;
      if (step == 12) {
        if (flg == 0) break;
        printf("    continue\n");
        flg = 0;
        step = 0;
      }
    }
  }
  printf("----rotated to L(%5d, %5d, %5d)--R(%5d, %5d, %5d) %d--->%d\n", l0, l1, l2, r0, r1, r2, ini, old);
  if (argvflg) cbout("l0.out" , 50, l0, l1, l2, r0, r1, r2, 10,  0,  0,  0,  0,  0);
  if (argvflg) cbout("l1.out" , 50, l0, l1, l2, r0, r1, r2,  0, 10,  0,  0,  0,  0);
  if (argvflg) cbout("l2.out" , 50, l0, l1, l2, r0, r1, r2,  0,  0, 10,  0,  0,  0);
  if (argvflg) cbout("r0.out" , 50, l0, l1, l2, r0, r1, r2,  0,  0,  0, 10,  0,  0);
  if (argvflg) cbout("r1.out" , 50, l0, l1, l2, r0, r1, r2,  0,  0,  0,  0, 10,  0);
  if (argvflg) cbout("r2.out" , 50, l0, l1, l2, r0, r1, r2,  0,  0,  0,  0,  0, 10);
  if (argvflg) cbout("l01.out", 50, l0, l1, l2, r0, r1, r2, 10, 10,  0,  0,  0,  0);
  if (argvflg) cbout("l12.out", 50, l0, l1, l2, r0, r1, r2,  0, 10, 10,  0,  0,  0);
  if (argvflg) cbout("l20.out", 50, l0, l1, l2, r0, r1, r2, 10,  0, 10,  0,  0,  0);
  if (argvflg) cbout("r01.out", 50, l0, l1, l2, r0, r1, r2,  0,  0,  0, 10, 10,  0);
  if (argvflg) cbout("r12.out", 50, l0, l1, l2, r0, r1, r2,  0,  0,  0,  0, 10, 10);
  if (argvflg) cbout("r20.out", 50, l0, l1, l2, r0, r1, r2,  0,  0,  0, 10,  0, 10);
  /*-----------------------*/
  DDDD = 1;
  Max_depth = Max_disp/2;
  Min_depth = Min_disp/2;
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = YY;
  SIZE_W = XX;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  rotate(l0, l1, l2, r0, r1, r2);
  if (argvflg) cv::imwrite("rec_R.png", rrframe);
  if (argvflg) cv::imwrite("rec_L.png", llframe);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Right, writer);
  if (argvflg) for (int i = 0; i < 10; i++) disp_and_write(Left, writer);
  graph_cut(penalty, penalty, inhibit, inhibit);
  if (argvflg) for (int i = 0; i < 200; i++) {
    disp_and_write(Stereo, writer);
    target_x -= 0.01;
  }
  if (argvflg) for (int i = 0; i < 200; i++) {
    disp_and_write(Stereo, writer);
    target_x += 0.01;
  }
  printf("Calibration completed\n");
  writer.release();
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
    printf("M: do calibration\n");
    printf("m: left picture\n");
    printf(",: stereo picture\n");
    printf(".: right picture\n");
    printf("<: both picture\n");
    printf("s: depth(disparity) to front\n");
    printf("t: depth(disparity) to back\n");
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
    return;
  }
  /*------------------------------*/
  if (key == 'M') {
    calib();
    return;
  }
  /*------------------------------*/
  if (key == 'm') {
    disp_mode = Left;
    glutPostRedisplay();
    return;
  }
  if (key == ',') {
    disp_mode = Stereo;
    glutPostRedisplay();
    return;
  }
  if (key == '.') {
    disp_mode = Right;
    glutPostRedisplay();
    return;
  }
  if (key == '<') {
    disp_mode = Both;
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 's') {
    depth_num++;
    if (depth_num > Max_disp/2) depth_num = Max_disp/2;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num);
    glutPostRedisplay();
    return;
  }
  if (key == 't') {
    depth_num--;
    if (depth_num < Min_disp/2) depth_num = Min_disp/2;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num);
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
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 9) {
    printf("Usage: './program_name right.ppm left.ppm max_disp min_disp penalty inhibit divisor argvflg'\n");
    return 1;
  }
  const char *rname = argv[1];
  const char *lname = argv[2];
  Max_disp = atoi(argv[3]);
  Min_disp = atoi(argv[4]);
  penalty = atoi(argv[5]);
  inhibit = atoi(argv[6]);
  divisor = atoi(argv[7]);
  argvflg = atoi(argv[8]);
  rframe = cv::imread(rname); if (rframe.empty()) { printf("cannot read %s\n", rname); return 1; }
  lframe = cv::imread(lname); if (lframe.empty()) { printf("cannot read %s\n", lname); return 1; }
  if ((rframe.cols != lframe.cols)
    ||(rframe.rows != lframe.rows)) {
    printf("Two images's sizes are not equal.\n");
    return 1;
  }
  XX = rframe.cols;
  YY = rframe.rows;
  DEP = new int[XX*YY];
  Max_depth = Max_disp/2;
  Min_depth = Min_disp/2;
  depth_num = Min_disp/2;
  DDDD = 1;
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = YY;
  SIZE_W = XX;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;

  half_base = HALF_BASE;
  focal = FOCAL;
  pixel = PIXEL;
  look_at_r = LOOK_AT_R;
  look_at_1 = LOOK_AT_1;
  look_at_2 = LOOK_AT_2;
  target_x = TARGET_X;
  target_y = TARGET_Y;
  target_z = TARGET_Z;
  disp_mode = Right;

  M = cv::Mat::zeros(3, 3, CV_64F);
  M.at<double>(0, 0) = focal/pixel;
  M.at<double>(0, 2) = double(XX-1)/2;
  M.at<double>(1, 1) = focal/pixel;
  M.at<double>(1, 2) = double(YY-1)/2;
  M.at<double>(2, 2) = 1.0;
  D = cv::Mat::zeros(1, 5, CV_64F);
  S = cv::Size(XX, YY);

  rotate(0, 0, 0, 0, 0, 0);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (XX, YY);
  glutInitWindowPosition (390, 44);
  glutCreateWindow("Stereo Rectifier by K. OGURI 20180425");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(100);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();

  delete [] DEP;
  llframe.release();
  rrframe.release();
  lframe.release();
  rframe.release();
  return 0;
}
