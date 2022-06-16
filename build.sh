#!/usr/bin/env bash

DIR=$(pwd)


COMPILER="clang++"
CFLAGS="-Wall -Wextra -Wconversion -Wshadow -Wpedantic -std=c++17 -g -O0"
OFFWARNINGS="-Wno-unused-function -Wno-missing-braces -Wno-gnu-anonymous-struct"

SRC="$DIR/src/*.cpp"
${COMPILER} ${SRC} ${CFLAGS} ${OFFWARNINGS} -o Ray && ./Ray > image.ppm && open ./image.ppm && rm ./Ray