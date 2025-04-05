cmake_minimum_required(VERSION 3.7.2)
project(fod)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FONTDUMP_SOURCES fontdump.cpp)

add_executable(fontdump ${FONTDUMP_SOURCES})
if(MSVC)
  target_compile_options(fontdump PRIVATE /W4 /WX)
else()
  target_compile_options(fontdump PRIVATE -Wall -Werror)
endif()

