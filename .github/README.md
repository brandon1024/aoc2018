# Advent of Code 2018 Solutions

<img src="advent.png"/>

Here are my solutions for Advent of Code 2018. Each solution is written in plain C (C99), and build out of the box with no dependencies.

My primary goal was to keep each solution separate and consistent. Each solution is built as its own executable, and both parts of the puzzle are solved in the same executable. For simplicity, I opted to take all puzzle input from stdin. This makes it easy to redirect the puzzle input from any file, like so: `./aocd12 < input.in`.

Each solution can be built using CMake, but you can also build using `gcc` if you'd like.

I also decided I wasn't going for the top of the leaderboard. I wanted to challenge myself to design and build beautiful and efficient solutions to the problems, and follow good programming standards (for the most part I follow the [Git Coding Guidelines](https://github.com/git/git/blob/master/Documentation/CodingGuidelines)).

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