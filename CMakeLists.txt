cmake_minimum_required(VERSION 3.10)
project(SlidingHistogram)
add_executable(SlidingHistogram Histogram.cpp CyclingBuffer.cpp SlidingHistogramCalc.cpp OptimizedSlidingHistogramCalc.cpp Main.cpp)