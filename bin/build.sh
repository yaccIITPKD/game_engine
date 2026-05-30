#!/bin/bash

set -e

mkdir -p ./build

clang++ ./bin/build.cpp -o build/game -O0 -g3 -std=c++11 -pedantic -lX11 -lGL -lXrandr
