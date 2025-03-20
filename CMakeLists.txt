cmake_minimum_required(VERSION 3.16.3)
project(fod)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# Currently the only supported output is SDL2.
find_package(SDL2 REQUIRED)
#find_package(X11)

# C++14
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INSTALL_SRCS src/install.c
  src/common/compress.c src/common/compress.h
  src/common/bufio.c src/common/bufio.h
  src/common/hexdump.c)

set(GAME_SRCS src/main.c
  src/common/compress.c src/common/compress.h
  src/common/bufio.c src/common/bufio.h
  src/common/hexdump.c
  src/fileio.c src/fileio.h
  src/resource.c src/resource.h
  src/tables.c src/tables.h
  src/vga.c src/vga.h src/vga_sdl.c
  src/utils/sha1.c src/utils/sha1.h)

add_executable(installer ${INSTALL_SRCS})
target_include_directories(installer PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(MSVC)
  target_compile_options(installer PRIVATE /W4 /WX)
else()
  target_compile_options(installer PRIVATE -Wall -Werror)
endif()

add_executable(fod ${GAME_SRCS})
target_include_directories(fod PRIVATE ${CMAKE_SOURCE_DIR}/src ${SDL2_INCLUDE_DIRS})
target_link_libraries(fod PRIVATE ${SDL2_LIBRARIES})
if(MSVC)
  target_compile_options(fod PRIVATE /W4 /WX)
else()
  target_compile_options(fod PRIVATE -Wall -Werror)
endif()
