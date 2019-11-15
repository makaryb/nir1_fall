#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

GLFLAGS = -std=c++11 -arch sm_30 -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
CLFLAGS = -std=c++11             -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
LIBRARIES += -lopencv_core -lopencv_highgui -lopencv_videoio

.SUFFIXES:

all: 3d_gpu 3d_cpu

3d_gpu:        3d_stereo.cpp        graph_cut.cu  makefile
	nvcc -o $@ 3d_stereo.cpp        graph_cut.cu  $(GLFLAGS) $(LIBRARIES)

3d_cpu:        3d_stereo.cpp        graph_cut.cpp makefile
	c++  -o $@ 3d_stereo.cpp        graph_cut.cpp $(CLFLAGS) $(LIBRARIES)

3d_g: 3d_gpu
	./3d_gpu R000000_10.png L000000_10.png 68 0 14 1023 1

3d_c: 3d_cpu
	./3d_cpu R000000_10.png L000000_10.png 68 0 14 1023 1

clean:
	rm -f 3d_gpu
	rm -f 3d_cpu
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
