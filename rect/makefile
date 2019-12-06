#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.04.25 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

LFLAGS = -std c++11 -arch sm_30 -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`

.SUFFIXES:

all: rec

#////////////////////////////////////////////////////
do: rec
#	./rec 2_R.png 2_L.png 28 0 14 4095 4 0
	./rec 3_R.png 3_L.png 24 0 14 4095 4 0
#	./rec 7_R.png 7_L.png 36 0 14 4095 4 0

#////////////////////////////////////////////////////
rec:           rectify.cpp graph_cut.cu makefile
	nvcc -o $@ rectify.cpp graph_cut.cu $(LFLAGS)

#////////////////////////////////////////////////////
clean:
	rm -f rec
	rm -f result.avi
	rm -f rec_R.png
	rm -f rec_L.png
	rm -f *.out
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
