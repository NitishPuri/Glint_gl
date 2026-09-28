#!/usr/bin/env bash
# Build and run. Usage: ./run.sh [Debug|Release]
set -e
cd "$(dirname "$0")"
TYPE=${1:-Debug}
./build.sh "$TYPE"
./build/$TYPE/Glint
