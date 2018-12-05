# Advent of Code 2018 Solutions
Here are my solutions for Advent of Code 2018. Each solution is written in plain C (C99) with no dependencies. Each solution can be built using CMake.

## Build
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Run
All solutions take input from stdin. If an input is provided in the problem, it will be included as a file named `input.in`.
```
cd build/day#
./aocd# < input.in
```