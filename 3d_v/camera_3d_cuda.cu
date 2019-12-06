/********************************************************************/
/**************************************** 2018/01/30 *** K. Oguri ***/
/********************************************************************/
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <cuda_gl_interop.h>
#include <GL/freeglut.h>
#include <opencv2/highgui/highgui.hpp>

#define XX 640
#define YY 480
#define PI (3.141592653589793238462643383279502884197f)
#define LOOK_AT_R (4.0f)
#define LOOK_AT_1 (PI/6)
#define LOOK_AT_2 (PI/6)

#define TD_NUM   (256)
#define FRM_NUM (YY*XX*3)

#define PADR(Y, X, C) \
 ((Y)*3*XX+\
  (X)*3+\
  (C))

float look_at_r;
float look_at_1;
float look_at_2;

struct vvv {
  float x;
  float y;
  float z;
  float r;
  float g;
  float b;
};
cudaGraphicsResource *v_res;

int *h_FRM;
int *d_FRM;
__constant__ int *FRM;
size_t FRM_SIZE = sizeof(int) * FRM_NUM;

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
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(vvv), NULL);
  glColorPointer(3, GL_FLOAT, sizeof(vvv), (GLvoid *)(sizeof(float)*3));
  glPointSize(1);
  glDrawArrays(GL_POINTS, 0, XX*YY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
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

__global__ void picture(vvv *pos) {
  int index = blockDim.x * blockIdx.x + threadIdx.x;
  if (index >= (XX*YY)) return;
  int hh  = index / XX;
  int ww  = index % XX;
  int h = hh - (YY/2);
  int w = ww - (XX/2);
  pos[index].x =  0.0f;
  pos[index].y =  0.001*w;
  pos[index].z = -0.001*h;
  pos[index].r = (float)FRM[PADR(hh, ww, 0)]/255;
  pos[index].g = (float)FRM[PADR(hh, ww, 1)]/255;
  pos[index].b = (float)FRM[PADR(hh, ww, 2)]/255;
}

void set_frm(cv::Mat frame) {
  for (int hh = 0; hh < YY; hh++) {
    for (int ww = 0; ww < XX; ww++) {
      cv::Vec3b bgr = frame.at<cv::Vec3b>(hh, ww);
      h_FRM[PADR(hh, ww, 0)] = bgr[2];
      h_FRM[PADR(hh, ww, 1)] = bgr[1];
      h_FRM[PADR(hh, ww, 2)] = bgr[0];
    }
  }
}

void idle(void) {
  *cap >> frame;
  /*------------------------------*/
  set_frm(frame);
  cudaMemcpy(d_FRM, h_FRM, FRM_SIZE, cudaMemcpyHostToDevice);
  cudaGraphicsMapResources(1, &v_res, NULL);
  vvv *pos;
  cudaGraphicsResourceGetMappedPointer((void**)&pos, NULL, v_res);
  picture<<< ((XX*YY)/TD_NUM)+1, TD_NUM >>>(pos);
  cudaGraphicsUnmapResources(1, &v_res, NULL);
  /*------------------------------*/
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
  int gpuCount = 0;
  cudaGetDeviceCount(&gpuCount);
  if (gpuCount == 0) { printf("There are no GPU.\n"); return 1; }
  cudaSetDevice(0);
  cudaDeviceProp gpuProp;
  cudaGetDeviceProperties(&gpuProp, 0);
  printf("GPU %s is used.\n", gpuProp.name);
  h_FRM = new int[FRM_NUM];
  cudaMalloc((void **)&d_FRM, FRM_SIZE);
  cudaMemcpyToSymbol(FRM, &d_FRM, sizeof(int*));
  cudaGLSetGLDevice(0);
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
  glewInit();
  GLuint v_buf;
  glGenBuffers(1, &v_buf);
  glBindBuffer(GL_ARRAY_BUFFER, v_buf);
  glBufferData(GL_ARRAY_BUFFER, (XX*YY)*sizeof(vvv), NULL, GL_DYNAMIC_DRAW);
  cudaGraphicsGLRegisterBuffer(&v_res, v_buf, cudaGraphicsMapFlagsNone);
  /*------------------------------*/
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(1.0);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();
  /*------------------------------*/
  cudaGraphicsUnregisterResource(v_res);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &v_buf);
  cudaDeviceReset();
  /*------------------------------*/
  frame.release();
  cap->release();
  delete cap;
  cudaFree(d_FRM);
  delete [] h_FRM;
  return 0;
}
