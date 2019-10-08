FLAGS = -std=c++11 -O3 `pkg-config opencv --cflags --libs`

.SUFFIXES:

###########################################################################################
do: compare
	./compare true.png tsu-ICM.png
	./compare true.png tsu-BP-S.png
	./compare true.png tsu-BP-M.png
	./compare true.png tsu-Expansion.png
	./compare true.png tsu-Swap.png
	./compare true.png tsu-TRW-S.png
	./compare true.png disparity14.png
	./compare true.png pen_14_inh_1023.png
	./compare true.png tsukuba-truedispL.png

###########################################################################################
compare: compare_disparity.cpp makefile
	g++ -o $@ compare_disparity.cpp $(FLAGS)

###########################################################################################
clean:
	rm -f compare
	rm -f *.stackdump
	-rm -i baka* kaba* aho* log*
