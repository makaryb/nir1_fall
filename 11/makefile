#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.04.25 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

CFLAGS = -std c++11 -arch sm_30 -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
LFLAGS =                        -O3                       `pkg-config opencv --cflags --libs`
LIBRARIES += -lopencv_core -lopencv_highgui -lopencv_videoio

.SUFFIXES:

all: rectify calib_png calib_cam depth_gpu

#////////////////////////////////////////////////////
rec: rectify
	./rectify 2_R.png 2_L.png 28 0 14 4095 4 8
#	./rectify 3_R.png 3_L.png 24 0 14 4095 4 8
#	./rectify 7_R.png 7_L.png 36 0 14 4095 4 8
#	./rectify playg_r.jpg playg_l.jpg 56 0 50 4095 4 13
#	./rectify playground_r.jpg playground_l.jpg 32 0 50 4095 2 13

dep: depth_gpu
#	./depth_gpu rec_R.png rec_L.png 56 0 50 4095 1 5
#	./depth_gpu rec_R.png rec_L.png 32 0 50 4095 1 5

near: calib_png
	./calib_png 9 6 0.0275f near.png

far: calib_png
	./calib_png 9 6 0.0275f far.png

ng: calib_png
	./calib_png 9 6 0.0275f board_ng.png

ok: calib_png
	./calib_png 9 6 0.0275f board_ok.png

cam: calib_cam
	./calib_cam 9 6 0.0275f

#////////////////////////////////////////////////////
rectify:       rectify.cpp graph_cut.cu makefile
	nvcc -o $@ rectify.cpp graph_cut.cu $(CFLAGS)

calib_png:    calib_png.cpp calib.cpp makefile
	g++ -o $@ calib_png.cpp calib.cpp $(LFLAGS) $(LIBRARIES)

calib_cam:    calib_cam.cpp calib.cpp makefile
	g++ -o $@ calib_cam.cpp calib.cpp $(LFLAGS) $(LIBRARIES)

depth_gpu:     depth_stereo.cpp graph_cut.cu makefile
	nvcc -o $@ depth_stereo.cpp graph_cut.cu $(CFLAGS)

#////////////////////////////////////////////////////
clean:
	rm -f rectify
	rm -f calib_png
	rm -f calib_cam
	rm -f depth_gpu
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
