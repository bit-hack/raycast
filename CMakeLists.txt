cmake_minimum_required(VERSION 2.8)
project(raycast)

add_executable(raycast main.cpp)
target_link_libraries(raycast SDL)
