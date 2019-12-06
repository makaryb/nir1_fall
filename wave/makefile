#////////////////////////////////////////////////////
#// Copyright (c) 2018 Kiyoshi Oguri    2018.04.25 //
#// Released under the MIT license                 //
#// http://opensource.org/licenses/mit-license.php //
#////////////////////////////////////////////////////

CFLAGS = -std c++11 -arch sm_30 -O3 -lm -lglut -lGLU -lGL `pkg-config opencv --cflags --libs`

.SUFFIXES:

all: rectify

#////////////////////////////////////////////////////
do: rectify
	./rectify 3_rec_R.png 3_rec_L.png 64 0 14 4095 1 0

#////////////////////////////////////////////////////
rectify:       rectify.cpp graph_cut.cu makefile
	nvcc -o $@ rectify.cpp graph_cut.cu $(CFLAGS)

#////////////////////////////////////////////////////
clean:
	rm -f rectify
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
