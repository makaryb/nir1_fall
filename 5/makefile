LFLAGS = -O3 -lm `pkg-config opencv --cflags --libs`
LIBRARIES += -lopencv_core -lopencv_highgui -lopencv_videoio

.SUFFIXES:

all: camera cameras

camera: camera.cpp
	g++ -o $@ camera.cpp $(LFLAGS)

cameras: cameras.cpp
	g++ -o $@ cameras.cpp $(LFLAGS) $(LIBRARIES)

one: camera
	./camera

two: cameras
	./cameras

clean:
	rm -f camera cameras
