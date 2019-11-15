CLFLAGS = -std=c++11             -O3 -lm -lGLEW -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
GLFLAGS = -std=c++11 -arch sm_30 -O3 -lm -lGLEW -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
LIBRARIES += -lopencv_core -lopencv_highgui -lopencv_videoio

.SUFFIXES:

all: camera_3d_glut camera_3d_glew camera_3d_cuda

glut: camera_3d_glut
	./camera_3d_glut

glew: camera_3d_glew
	./camera_3d_glew

cuda: camera_3d_cuda
	./camera_3d_cuda

camera_3d_glut: camera_3d_glut.cpp makefile
	g++ -o $@ camera_3d_glut.cpp $(CLFLAGS) $(LIBRARIES)

camera_3d_glew: camera_3d_glew.cpp makefile
	g++ -o $@ camera_3d_glew.cpp $(CLFLAGS) $(LIBRARIES)

camera_3d_cuda: camera_3d_cuda.cu makefile
	nvcc -o $@ camera_3d_cuda.cu $(GLFLAGS) $(LIBRARIES)

clean:
	rm -f camera_3d_glut
	rm -f camera_3d_glew
	rm -f camera_3d_cuda
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
