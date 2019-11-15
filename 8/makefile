#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

CLFLAGS1 = -std=c++11 -O3 -lpng
CLFLAGS2 = -std=c++11 -O3 -lm -lpng -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`
GLFLAGS = -std=c++11  -O3 -lpng `pkg-config opencv --cflags --libs`
LIBRARIES += -lopencv_core -lopencv_highgui -lopencv_videoio

.SUFFIXES:

all: depth true 3d cmp chk training submit

#////////////////////////////////////////////////////
do1: depth
	./depth R000000_10.png L000000_10.png 70 4 24 4095

do2: true
	./true N000000_10.png          N000000_10_true.png N000000_10_error.png
#	./true N000000_10_true.png     true.png            error.png
#	./true dis_pen_24_inh_4095.png true.png            error.png

do3: 3d
	./3d R000000_10.png L000000_10.png 70 4 dep_pen_24_inh_4095.png N000000_10_true.png

do4: cmp
	./cmp N000000_10_true.png dis_pen_24_inh_4095.png error.png

do5: chk
	./chk

do6: training
	./training

do7: submit
	./submit

#////////////////////////////////////////////////////
depth:         depth_kitti.cpp graph_cut.cpp makefile
	g++ -o $@ depth_kitti.cpp graph_cut.cpp $(GLFLAGS) $(LIBRARIES)

true:         true_kitti.cpp makefile
	g++ -o $@ true_kitti.cpp $(CLFLAGS1) $(LIBRARIES)

3d:           3d_kitti.cpp makefile
	g++ -o $@ 3d_kitti.cpp $(CLFLAGS2) $(LIBRARIES)

cmp:          cmp_kitti.cpp makefile
	g++ -o $@ cmp_kitti.cpp $(CLFLAGS1) $(LIBRARIES)

chk:          chk_kitti.cpp makefile
	g++ -o $@ chk_kitti.cpp $(CLFLAGS1) $(LIBRARIES)

training:      training_kitti.cpp graph_cut.cpp makefile
	g++ -o $@ training_kitti.cpp graph_cut.cpp $(GLFLAGS) $(LIBRARIES)

submit:        submit_kitti.cpp graph_cut.cpp makefile
	g++ -o $@ submit_kitti.cpp graph_cut.cpp $(GLFLAGS) $(LIBRARIES)

#////////////////////////////////////////////////////
clean:
	rm -f depth
	rm -f true
	rm -f 3d
	rm -f cmp
	rm -f chk
	rm -f training
	rm -f submit
	rm -f true.png error.png
	rm -f *.stackdump
	-rm -i dep_pen_24_inh_4095.png
	-rm -i dis_pen_24_inh_4095.png
	-rm -i N000*_10_true.png
	-rm -i N000*_10_error.png
	-rm -i baka* kaba* aho* log*
