/********************************************************************/
/**************************************** 2018/01/30 *** K. Oguri ***/
/********************************************************************/
#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <opencv2/highgui/highgui.hpp>

#define XX 640
#define YY 480
#define PI (3.141592653589793238462643383279502884197f)
#define LOOK_AT_R (4.0f)
#define LOOK_AT_1 (PI/6)
#define LOOK_AT_2 (PI/6)

float look_at_r;
float look_at_1;
float look_at_2;

cv::Mat frame(YY, XX, CV_8UC3, cv::Scalar(0, 0, 0));
cv::VideoCapture *cap;

/********************************************************************/
void display(void) {
  float p = look_at_r * cos(look_at_1);
  float z = look_at_r * sin(look_at_1);
  float x =         p * cos(look_at_2);
  float y =         p * sin(look_at_2);
  glLoadIdentity();
  if (p >= 0.0) gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
  else          gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 0.0,-1.0);
  /*---------------------------*/
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  /*---------------------------*/
  glPointSize(1);
  glBegin(GL_POINTS);
  for (int hh = 0; hh < YY; hh++) {
    for (int ww = 0; ww < XX; ww++) {
      int h = hh - (YY/2);
      int w = ww - (XX/2);
      cv::Vec3b bgr = frame.at<cv::Vec3b>(hh, ww);
      glColor3f((float)bgr[2]/255, (float)bgr[1]/255, (float)bgr[0]/255);
      glVertex3f(0.0f, 0.001*w, -0.001*h);
    }
  }
  glEnd();
  /*---------------------------*/
  glColor3f(1.0, 0.0, 0.0); glBegin(GL_LINES); glVertex3f(-1.0, 0.0, 0.0); glVertex3f(1.0, 0.0, 0.0); glEnd();
  glColor3f(0.0, 1.0, 0.0); glBegin(GL_LINES); glVertex3f( 0.0,-1.0, 0.0); glVertex3f(0.0, 1.0, 0.0); glEnd();
  glColor3f(0.0, 0.0, 1.0); glBegin(GL_LINES); glVertex3f( 0.0, 0.0,-1.0); glVertex3f(0.0, 0.0, 1.0); glEnd();
  /*---------------------------*/
  glDisable(GL_DEPTH_TEST);
  glutSwapBuffers();
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30, (float)w / h, 0.001, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') {
    glutLeaveMainLoop();
    return;
  }
  if (key == 'h') { look_at_2 -= PI / 90; return; }
  if (key == 'j') { look_at_1 -= PI / 90; return; }
  if (key == 'k') { look_at_1 += PI / 90; return; }
  if (key == 'l') { look_at_2 += PI / 90; return; }
  if (key == 'f') { look_at_r /= 0.9f; return; }
  if (key == 'n') { look_at_r *= 0.9f; return; }
}

void idle(void) {
  *cap >> frame;
  glutPostRedisplay();
}

/********************************************************************/
int main(int argc, char** argv) {
  look_at_r = LOOK_AT_R;
  look_at_1 = LOOK_AT_1;
  look_at_2 = LOOK_AT_2;
  /*------------------------------*/
  cap = new cv::VideoCapture;
  cap->open(0);
  if (cap->isOpened()) printf("Camera Successfully Opened\n");
  else               { printf("Cannot Open Camera\n"); return 1; }
  /*------------------------------*/
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (2*XX, 2*YY);
  glutInitWindowPosition (390, 44);
  glutCreateWindow("K. OGURI 20180130");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);
  /*------------------------------*/
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(1.0);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();
  /*------------------------------*/
  frame.release();
  cap->release();
  delete cap;
  return 0;
}
