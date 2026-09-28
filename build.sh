#!/usr/bin/env bash
# Configure (first time) and build. Usage: ./build.sh [Debug|Release]
set -e
cd "$(dirname "$0")"
TYPE=${1:-Debug}
cmake -S . -B build/$TYPE -G Ninja -DCMAKE_BUILD_TYPE=$TYPE
cmake --build build/$TYPE
