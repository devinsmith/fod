cmake_minimum_required(VERSION 3.7.2)
project(fod)
set(TARGET_NAME common)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled: Adding -fsanitize=address,undefined")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

# C and C++ sources are freely mixed.
set(SOURCES bufio.c bufio.h
  compress.c compress.h
  fileio.c fileio.h
  game.c game.h
  hexdump.c hexdump.h
  random.c random.h
  resource.c resource.h
  sha1.c sha1.h
  tables.c tables.h
  ui.c ui.h
  vga.c vga.h)

add_library(${TARGET_NAME} STATIC ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(MSVC)
  # Not yet with /WX
  #target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
  target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
  # Not yet with -Werror
  #target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
  target_compile_options(${TARGET_NAME} PRIVATE -Wall)
endif()
