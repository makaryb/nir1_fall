#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

GLFLAGS = -std=c++11 -arch sm_30 -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
CLFLAGS = -std=c++11             -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`

.SUFFIXES:

all: disparity_gpu disparity_cpu depth_gpu depth_cpu 3d_gpu 3d_cpu

disparity_gpu: disparity_stereo.cpp graph_cut.cu  makefile
	nvcc -o $@ disparity_stereo.cpp graph_cut.cu  $(GLFLAGS)

disparity_cpu: disparity_stereo.cpp graph_cut.cpp makefile
	c++  -o $@ disparity_stereo.cpp graph_cut.cpp $(CLFLAGS)

depth_gpu:     depth_stereo.cpp     graph_cut.cu  makefile
	nvcc -o $@ depth_stereo.cpp     graph_cut.cu  $(GLFLAGS)

depth_cpu:     depth_stereo.cpp     graph_cut.cpp makefile
	c++  -o $@ depth_stereo.cpp     graph_cut.cpp $(CLFLAGS)

3d_gpu:        3d_stereo.cpp        graph_cut.cu  makefile
	nvcc -o $@ 3d_stereo.cpp        graph_cut.cu  $(GLFLAGS)

3d_cpu:        3d_stereo.cpp        graph_cut.cpp makefile
	c++  -o $@ 3d_stereo.cpp        graph_cut.cpp $(CLFLAGS)

dis_g: disparity_gpu
	./disparity_gpu scene1.row3.col3.ppm scene1.row3.col1.ppm 28 10 14 1023 8

dis_c: disparity_cpu
	./disparity_cpu scene1.row3.col3.ppm scene1.row3.col1.ppm 28 10 14 1023 8

dep_g: depth_gpu
	./depth_gpu scene1.row3.col3.ppm scene1.row3.col1.ppm 28 10 14 1023 1 8

dep_c: depth_cpu
	./depth_cpu im1.png im0.png 28 10 14 1023 1 8

3d_g: 3d_gpu
	./3d_gpu scene1.row3.col3.ppm scene1.row3.col1.ppm 28 10 14 1023 1

3d_c: 3d_cpu
	./3d_cpu scene1.row3.col3.ppm scene1.row3.col1.ppm 28 10 14 1023 1

clean:
	rm -f disparity_gpu
	rm -f disparity_cpu
	rm -f depth_gpu
	rm -f depth_cpu
	rm -f 3d_gpu
	rm -f 3d_cpu
	rm -f pen*.png
	rm -f disparity*.png
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
