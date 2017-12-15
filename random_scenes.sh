#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: $0 <number_of_scenes>"
	exit
fi

for i in $(seq 1 $1)
do
	build/raytracer -R -t 0.5 -m 0.35 -b 0.15 -n 40 --radius-max 10 > "scenes/random$i.scn"
	build/raytracer -H 500 -W 500 -b 15 -r 10 --fov 70 -f "scenes/random$i.scn" -o "generated/random$i.bmp"
done
