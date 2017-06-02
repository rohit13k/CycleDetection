cmake_minimum_required(VERSION 3.7)
project(CycleDetection)

set(CMAKE_CXX_STANDARD 14)
include_directories("D:/code/tclap-1.2.1/include")
set(SOURCE_FILES main.cpp countCycleFrequency.cpp countCycleFrequency.h)
add_executable(CycleDetection ${SOURCE_FILES})