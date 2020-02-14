cmake_minimum_required(VERSION 3.14)
project(testing)

set(CMAKE_CXX_STANDARD 14)

include_directories(./../src)

add_executable(testing main.cpp ../src/FastPathBP.h ../src/FastPathBP.cc ../src/branch_predictor.h)
