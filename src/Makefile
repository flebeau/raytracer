CC=g++
LDFLAGS=-L/usr/X11R6/lib -lm -lpthread -lX11
CFLAGS=-std=c++11
DEPS = vector.hpp ray.hpp sphere.hpp scene.hpp utils.hpp CImg.h material.hpp
OBJ = vector.o sphere.o scene.o

all: raytracer

vector.o: vector.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sphere.o: sphere.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scene.o: scene.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer: raytracer.cpp $(OBJ) $(DEPS)
	$(CC) -o $@ raytracer.cpp $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o
