cmake_minimum_required(VERSION 3.16.3)
project(fod)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# Currently the only supported output is SDL2.
find_package(SDL2 REQUIRED)
#find_package(X11)

# C++14
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
