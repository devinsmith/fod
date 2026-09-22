cmake_minimum_required(VERSION 3.16.3)

project(fod)
set(TARGET_NAME fod)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled: Adding -fsanitize=address,undefined")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

include(CheckFunctionExists)
if(NOT FLOOR_FUNCTION_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)
  CHECK_FUNCTION_EXISTS(floor FLOOR_FUNCTION_EXISTS)
  if(NOT FLOOR_FUNCTION_EXISTS)
    unset(FLOOR_FUNCTION_EXISTS CACHE)
    list(APPEND CMAKE_REQUIRED_LIBRARIES m)
    CHECK_FUNCTION_EXISTS(floor FLOOR_FUNCTION_EXISTS)
    if(FLOOR_FUNCTION_EXISTS)
      set(NEED_LINKING_AGAINST_LIBM True CACHE BOOL "" FORCE)
    else()
      message(FATAL_ERROR "Failed making the floor() function available")
    endif()
  endif()
endif()


# C and C++ sources are freely mixed.
set(SOURCES main.c plat_sdl.c)

add_executable(${TARGET_NAME} ${SOURCES})
list(APPEND INC_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LINK_LIBRARIES ${SDL2_LIBRARIES})

if (NEED_LINKING_AGAINST_LIBM)
  list(APPEND LINK_LIBRARIES m)
endif()


target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib ${INC_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE common ${LINK_LIBRARIES})

install(TARGETS fod DESTINATION bin)

if(MSVC)
    # Not yet with /WX
    #target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
    # Not yet with -Werror
    #target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
    target_compile_options(${TARGET_NAME} PRIVATE -Wall)
endif()
