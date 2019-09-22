/********************************************************************/
/***** 3D Stereo Vision using Gaze_line-Depth_number Model **********/
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
#include <math.h>
#include <GL/freeglut.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat rframe;
cv::Mat lframe;

int *DEP;

#define IADR(H, W) \
 ((H)*SIZE_W+\
  (W))

#define PI (3.141592653589793238462643383279502884197)
#define HALF_BASE (0.05)
#define FOCAL (0.0814)
#define PIXEL (0.0001)
#define LOOK_AT_R (1.0)
#define LOOK_AT_1 (0.0)
#define LOOK_AT_2 (0.0)
#define Right (0)
#define Left (1)
#define Stereo (2)

double half_base;
double focal;
double pixel;
double look_at_r;
double look_at_1;
double look_at_2;
int penalty;
int inhibit;
int depth_num;
int disp_mode;
int figure;

void call_rgb(cv::Mat &frame, int y, int x, int *r, int *g, int *b) {
  if (x < 0) x = 0;
  if (x > SIZE_W-1) x = SIZE_W-1;
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

void display_hypo(void) {
  glPointSize(1);
  glBegin(GL_POINTS);
  for (int h = -SIZE_H/2; h < SIZE_H/2; h++) {
    int hh = h + SIZE_H/2;
    for (int w = -SIZE_W/2; w < SIZE_W/2; w++) {
      int ww = w + SIZE_W/2;
      int d = (disp_mode == Stereo)? DEP[IADR(hh, ww)]: depth_num;
      int r = w-d;
      int l = w+d;
      double x = -2*focal*half_base/((l-r)*pixel);
      double y =  half_base*(l+r)/(l-r);
      double z = -2*h*half_base/(l-r);
      int RR, RG, RB;
      int LR, LG, LB;
      double R, G, B;
      if (disp_mode == Stereo) {
        call_rgb(rframe, hh, ww-d, &RR, &RG, &RB);
        call_rgb(lframe, hh, ww+d, &LR, &LG, &LB);
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
      else { }
      glColor3d(R, G, B);
      glVertex3d(x, y, z);
    }
  }
  glEnd();
}

void display_all(void) {
  glEnable(GL_DEPTH_TEST);
  if (figure) {
    int r = -depth_num;
    int l = +depth_num;
    double shift_x = -2*focal*half_base/((l-r)*pixel);
    double shift_y =  half_base*(l+r)/(l-r);
    /*---------------------------*/
    glPointSize(4);
    glColor3d(1.0, 0.0, 1.0);
    /*---------------------------*/
    glBegin(GL_POINTS);
    glVertex3d(0.0,  half_base, 0.0);
    glVertex3d(0.0, -half_base, 0.0);
    glEnd();
    /*---------------------------*/
    glBegin(GL_LINES);
    glVertex3d(0.0,  half_base, 0.0);
    glVertex3d(shift_x, shift_y, 0.0);
    glEnd();
    /*---------------------------*/
    glBegin(GL_LINES);
    glVertex3d(0.0, -half_base, 0.0);
    glVertex3d(shift_x, shift_y, 0.0);
    glEnd();
    /*---------------------------*/
    glPointSize(1);
    glBegin(GL_POINTS);
    for (int h = -SIZE_H/2; h < SIZE_H/2; h++) {
      int hh = h + SIZE_H/2;
      for (int w = -SIZE_W/2; w < SIZE_W/2; w++) {
        int ww = w + SIZE_W/2;
        int R, G, B;
        /*---------------------------*/
        call_rgb(rframe, hh, ww, &R, &G, &B);
        glColor3d((double)R/255, (double)G/255, (double)B/255);
        glVertex3d(-focal, half_base+w*pixel, -h*pixel);
        /*---------------------------*/
        call_rgb(lframe, hh, ww, &R, &G, &B);
        glColor3d((double)R/255, (double)G/255, (double)B/255);
        glVertex3d(-focal, -half_base+w*pixel, -h*pixel);
      }
    }
    glEnd();
  }
  display_hypo();
  glDisable(GL_DEPTH_TEST);
}

void display(void) {
  double p = look_at_r * cos(look_at_1);
  double z = look_at_r * sin(look_at_1);
  double x =         p * cos(look_at_2);
  double y =         p * sin(look_at_2);
  glLoadIdentity();
  if (p >= 0.0) gluLookAt(x, y, z, 0, 0, 0, 0, 0,  1);
  else          gluLookAt(x, y, z, 0, 0, 0, 0, 0, -1);
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
    printf("e: toggle figure mode\n");
    printf("m: left picture\n");
    printf(",: stereo picture (do graph_cut)\n");
    printf(".: right picture\n");
    printf("s: depth(disparity) to front\n");
    printf("t: depth(disparity) to back\n");
    printf("h: eye move left\n");
    printf("j: eye move down\n");
    printf("k: eye move up\n");
    printf("l: eye move right\n");
    printf("f: eye move far\n");
    printf("n: eye move near\n");
    printf("!: decrease base line\n");
    printf("@: increase base line\n");
    printf("#: decrease focal length\n");
    printf("$: increase focal length\n");
    printf("%%: decrease pixel size\n");
    printf("^: increase pixel size\n");
    printf("z: decrease penalty\n");
    printf("x: increase penalty\n");
    printf("Z: decrease inhibit\n");
    printf("X: increase inhibit\n");
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
  if (key == 'm') {
    disp_mode = Left;
    glutPostRedisplay();
    return;
  }
  if (key == ',') {
    disp_mode = Stereo;
    graph_cut(penalty, penalty, inhibit-1, inhibit-1);
    glutPostRedisplay();
    return;
  }
  if (key == '.') {
    disp_mode = Right;
    glutPostRedisplay();
    return;
  }
  /*------------------------------*/
  if (key == 's') {
    depth_num++;
    if (depth_num > Max_depth) depth_num = Max_depth;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num/strech);
    glutPostRedisplay();
    return;
  }
  if (key == 't') {
    depth_num--;
    if (depth_num < Min_depth) depth_num = Min_depth;
    printf("depth_num=%d(disparity=%d)\n", depth_num, 2*depth_num/strech);
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
  /*------------------------------*/
  if (key == 'z') {
    if (penalty > 0) {
      penalty--;
    }
    printf("penalty=%d\n", penalty);
    return;
  }
  if (key == 'x') {
    penalty++;
    printf("penalty=%d\n", penalty);
    return;
  }
  /*------------------------------*/
  if (key == 'Z') {
    if (inhibit > 1) {
      inhibit /= 2;
    }
    printf("inhibit=%d\n", inhibit-1);
    return;
  }
  if (key == 'X') {
    inhibit *= 2;
    printf("inhibit=%d\n", inhibit-1);
    return;
  }
}

/********************************************************************/
int main(int argc, char** argv) {
  if (argc != 8) {
    printf("Usage: './program_name right.ppm left.ppm max_disparity min_disparity penalty inhibit strech'\n");
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
  Max_depth = strech*atoi(argv[3])/2;
  Min_depth = strech*atoi(argv[4])/2;
  penalty = atoi(argv[5]);
  inhibit = atoi(argv[6]) + 1;
  resize(rframe, rframe, cv::Size(), strech, strech, cv::INTER_CUBIC);
  resize(lframe, lframe, cv::Size(), strech, strech, cv::INTER_CUBIC);
  SIZE_S = Max_depth-Min_depth;
  SIZE_H = rframe.rows;
  SIZE_W = rframe.cols;
  SIZE_HW = SIZE_H*SIZE_W;
  SIZE_SHW = SIZE_S*SIZE_H*SIZE_W;
  DEP = new int[SIZE_HW];

  half_base = HALF_BASE;
  focal = FOCAL;
  pixel = PIXEL/strech;
  look_at_r = LOOK_AT_R;
  look_at_1 = LOOK_AT_1;
  look_at_2 = LOOK_AT_2;
  depth_num = Min_depth;
  disp_mode = Right;
  figure = 1;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (2*SIZE_W/strech, 2*SIZE_H/strech);
  glutInitWindowPosition (390, 44);
  glutCreateWindow("Stereo Vision by K. OGURI 20180214");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(100);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();

  delete [] DEP;
  lframe.release();
  rframe.release();
  return 0;
}
