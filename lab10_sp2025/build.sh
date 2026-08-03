#1/bin/bash
# configure and compiles the project
# only need to run once at the start of the project
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
