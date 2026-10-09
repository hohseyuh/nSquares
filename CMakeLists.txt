cmake_minimum_required(VERSION 4.3)
project(nSquares)

set(CMAKE_CXX_STANDARD 26)

add_executable(nSquares main.cpp
        board.h
        board.cpp
        heuristics.h
        heuristics.cpp
        search.h
        search.cpp
        cli.h
        cli.cpp)
