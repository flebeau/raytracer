#!/bin/bash

mkdir -p generated

# Basic features with and without diffuse
for scene in scenes/concrete.scn scenes/transparent_sphere.scn scenes/mirror.scn
do
	name=$(basename "$scene" ".scn")
	if [ ! -f "generated/${name}_no_diffuse.bmp" ]
	then
		echo "No diffuse: ${name}"
		time build/raytracer -H 800 -W 800 -b 2 -r 1 --fov 60 -f "$scene" -o "generated/${name}_no_diffuse.bmp" --no-antialiasing --no-fresnel --no-diffuse
	fi
	if [ ! -f "generated/${name}_no_anti.bmp" ]
	then
		echo "Diffuse but no antialiasing: ${name}"
		time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 60 -f "$scene" -o "generated/${name}_no_anti.bmp" --no-antialiasing --no-fresnel
	fi
done

# Crazy
if [ ! -f "generated/crazy_no_diffuse.bmp" ]
then
	echo "Crazy sphere"
	time build/raytracer -H 800 -W 800 -b 2 -r 1 --no-antialiasing --no-fresnel --no-diffuse --deterministic -f scenes/crazy.scn -o generated/crazy_no_diffuse.bmp
fi

# Antialiasing and Fresnel
if [ ! -f "generated/concrete.bmp" ]
then
	echo "Concrete with antialiasing"
	time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 60 -f "scenes/concrete.scn" -o "generated/concrete.bmp"
fi
if [ ! -f "generated/transparent_sphere_no_fresnel.bmp" ]
then
	echo "Transparent sphere with no fresnel"
	time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 60 -f "scenes/transparent_sphere.scn" -o "generated/transparent_sphere_no_fresnel.bmp" --no-fresnel
fi
if [ ! -f "generated/transparent_sphere.bmp" ]
then
	echo "Transparent sphere with all functionalities"
	time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 60 -f "scenes/transparent_sphere.scn" -o "generated/transparent_sphere.bmp"
fi

# Inclusion of spheres and multicolor sphere
for scene in scenes/pellicule.scn scenes/glass_in_light_glass.scn scenes/multicolor.scn
do
	name=$(basename "$scene" ".scn")
	if [ ! -f "generated/${name}.bmp" ]
	then
		time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 60 -f "$scene" -o "generated/${name}.bmp"
	fi
done

# Random scene generator
for scene in scenes/interesting*.scn
do
	name=$(basename "$scene" ".scn")
	if [ ! -f "generated/${name}.bmp" ]
	then
		time build/raytracer -H 800 -W 800 -b 12 -r 1000 --fov 70 -f "$scene" -o "generated/${name}.bmp"
	fi
done
