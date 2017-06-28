cmake_minimum_required(VERSION 3.7)
project(CycleDetection)

set(CMAKE_CXX_STANDARD 14)
include_directories("/home/rohit/work/tclap-1.2.1/include/" )


set(SOURCE_FILES main.cpp countCycleFrequency.cpp countCycleFrequency.h countWithPath.h countWithPath.cpp DetectCycle.cpp DetectCycle.h FileIndexer.h FileIndexer.cpp MemoryMonitor.cpp MemoryMonitor.h)

add_executable(CycleDetection ${SOURCE_FILES})
