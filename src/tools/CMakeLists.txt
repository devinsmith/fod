cmake_minimum_required(VERSION 3.7.2)
project(fod)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FONTDUMP_SOURCES fontdump.cpp)
set(GANIDUMP_SOURCES ganidump.cpp)
set(UNPACK_SOURCES unpack.c)

add_executable(fontdump ${FONTDUMP_SOURCES})
if(MSVC)
  target_compile_options(fontdump PRIVATE /W4 /WX)
else()
  target_compile_options(fontdump PRIVATE -Wall -Werror)
endif()

add_executable(unpack ${UNPACK_SOURCES})
if(MSVC)
  target_compile_options(unpack PRIVATE /W4 /WX)
else()
  target_compile_options(unpack PRIVATE -Wall -Werror)
endif()

add_executable(ganidump ${GANIDUMP_SOURCES})
target_include_directories(ganidump PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_link_libraries(ganidump PRIVATE common)
if(MSVC)
  target_compile_options(ganidump PRIVATE /W4 /WX)
else()
  target_compile_options(ganidump PRIVATE -Wall -Werror)
endif()


