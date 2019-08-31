cmake_minimum_required(VERSION 2.8)
project(raycast)

file(GLOB SOURCE *.cpp *.h)

add_executable(raycast ${SOURCE})
target_link_libraries(raycast SDL)
