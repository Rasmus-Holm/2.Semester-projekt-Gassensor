#!/bin/sh

# Remove the existing build directory and its contents
rm -rf build

# Re-run cmake to generate fresh build files
# cmake -S . -B out/build
cmake -B build
