#!/bin/bash

cd linux/Box2D/Box2D/Build
make clean
rm CMakeCache.txt
cmake -DBOX2D_INSTALL=ON -DBOX2D_BUILD_SHARED=ON -DCMAKE_BUILD_TYPE=Release ..
make

