#!/bin/bash

set -e

mkdir -p build

time {
	if [ "$1" = "game" ]; then
		clang++ game/game.cpp -o game_temp.so -fPIC -shared
		mv game_temp.so build/game.so
		cp -rf res/* build/.
	else
		clang++ bin/build.cpp \
			-o build/game \
			-O0 -g3 -std=c++11 -pedantic \
			-lX11 -lXrandr -lGL

		clang++ game/game.cpp -o game_temp.so -fPIC -shared
		mv game_temp.so build/game.so
		cp -rf res/* build/.
	fi
}
