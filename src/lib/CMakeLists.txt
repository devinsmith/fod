cmake_minimum_required(VERSION 3.16.3)
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
set(SOURCES
  anim.cpp anim.h
  bufio.cpp bufio.h
  compress.cpp compress.h
  cmdmenu.cpp cmdmenu.h
  cursor.cpp cursor.h
  fileio.cpp fileio.h
  game.cpp game.h
  hexdump.cpp hexdump.h
  item.cpp item.h
  level.cpp level.h
  npc.cpp npc.h
  platform.cpp platform.h
  player.cpp player.h
  random.cpp random.h
  resource.cpp resource.h
  script.cpp script.h
  services.cpp services.h
  sfx.cpp sfx.h
  sha1.cpp sha1.h
  tables.cpp tables.h
  ui.cpp ui.h)

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
