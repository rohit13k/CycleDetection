# CycleDetection

### 1. To compile the code you will need
      a) Jetbeans Clion with cmake
      b) Gcc compiler
      c) tclap-1.2.1 library
### 2. The memory monitor code works only in Linux.


## To Compile:
### 1. Edit the CMakeList to update the location of tclap-1.2.1 library
### 2. If using the build tool in CLion then just use clean build
### 3. If using command line go to folder cmake-build-debug and run make

## To run:
### 1. If using CLion then use Run->Edit configuration->Program arguments and provide the following arguments
-i $full-path-of-input-graph-file$ -w $window-in-hour$ -o $output-file-location$ -p $batch size$ -r $bool:edge-revered$ -a $algorithm-no$ -b $bool:batchmode$ -z $bool:compress$

### 2. If using command line just run the exe with the above arguments
(Use sample script file in cmake-build-debug for details on various combinations of algorithms and parameters)
