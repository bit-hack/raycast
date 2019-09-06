cmake_minimum_required(VERSION 2.8)
project(raycast)

find_package(SDL REQUIRED)

file(GLOB SOURCE *.cpp *.h)

add_executable(raycast ${SOURCE})

include_directories(${SDL_INCLUDE_DIR})
target_link_libraries(raycast ${SDL_LIBRARY})

if (NOT UNIX)
  # We do this check to see if we have a 32-bit or 64-bit compiler. We can then
  # grab the correct SDL lib.
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    install (FILES ${SDL_INCLUDE_DIR}/../lib/x86/SDL.dll DESTINATION bin)
    configure_file(${SDL_INCLUDE_DIR}/../lib/x86/SDL.dll 
      ${PROJECT_BINARY_DIR}/SDL.dll COPYONLY)
  else()
    install (FILES ${SDL_INCLUDE_DIR}/../lib/x64/SDL.dll DESTINATION bin)
    configure_file(${SDL_INCLUDE_DIR}/../lib/x64/SDL.dll 
      ${PROJECT_BINARY_DIR}/SDL.dll COPYONLY)
  endif()
else()
  set(CMAKE_C_FLAGS "-march=nehalem")
  set(CMAKE_CXX_FLAGS "-march=nehalem")
endif()

# Handle the game assets

set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Copy the entire assets directory to the build folder
add_custom_command(TARGET raycast PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${ASSET_DIR} $<TARGET_FILE_DIR:raycast>/data/)

# Setup the install target
# TODO: "install" the files in a way that doesn't require you to list each one.
install (TARGETS raycast DESTINATION bin)
install (FILES ${ASSET_DIR}/walls/boxy.bmp DESTINATION bin/data/walls)
install (FILES ${ASSET_DIR}/floors/hex.bmp DESTINATION bin/data/floors)
