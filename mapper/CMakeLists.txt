file(GLOB SOURCE *.cpp)

include_directories(${SDL_INCLUDE_DIR})
add_executable(mapper ${SOURCE})
target_link_libraries(mapper ${SDL_LIBRARY})
