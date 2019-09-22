FLAGS = -std=c++11 -O3 `pkg-config opencv --cflags --libs`

.SUFFIXES:

###########################################################################################
do: true
	./true truedisp.row3.col3.pgm true.png 8

###########################################################################################
true: true_disparity.cpp makefile
	g++ -o $@ true_disparity.cpp $(FLAGS)

###########################################################################################
clean:
	rm -f true
	rm -f true.png
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
