cmake_minimum_required(VERSION 3.16)
project(epd7in5_v2_driver LANGUAGES CXX)
# The physical implementation is compiled by Arduino/PlatformIO. Desktop
# consumers share the API but link the simulator's implementation instead.
add_library(epd7in5_v2_api INTERFACE)
target_include_directories(epd7in5_v2_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
