#!/usr/bin/env bash
DIR=$(pwd)

COMPILER="clang++"
CFLAGS="-Wall -Wextra -Wconversion -Wshadow -Wpedantic -std=c++17 -g -O0 -fno-exceptions"
OFFWARNINGS="-Wno-unused-function -Wno-missing-braces -Wno-gnu-anonymous-struct"
COMPILER_MACROS_DEFINITIONS="-DDEBUG_BUILD=1"


SRC="$DIR/src/*.cpp"
${COMPILER} ${SRC} ${CFLAGS} ${OFFWARNINGS} ${COMPILER_MACROS_DEFINITIONS} -o Ray && ./Ray > MyImage.ppm && open ./MyImage.ppm && rm ./Ray

