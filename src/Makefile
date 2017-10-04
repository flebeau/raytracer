CC=g++
LDFLAGS=-L/usr/X11R6/lib -lm -lpthread -lX11
DEPS = vector.hpp ray.hpp sphere.hpp utils.hpp CImg.h
OBJ = vector.o sphere.o

all: raytracer

vector.o: vector.cpp $(DEPS)
	$(CC) -c -o $@ $<

sphere.o: sphere.cpp $(DEPS)
	$(CC) -c -o $@ $<

raytracer: raytracer.cpp $(OBJ) $(DEPS)
	$(CC) -o $@ raytracer.cpp $(OBJ) $(LDFLAGS)

clean:
	rm -rf *.o
