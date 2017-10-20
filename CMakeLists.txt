cmake_minimum_required( VERSION 3.1 )

project(Raytracer)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -L/usr/X11R6/lib")

include_directories(src)
file(GLOB SOURCES src/*.cpp)

# Packages
find_package(Boost COMPONENTS timer)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(raytracer ${SOURCES})
target_link_libraries(raytracer m ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
