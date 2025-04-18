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

# C and C++ sources are freely mixed.
set(SOURCES main.c vga_sdl.c)

add_executable(${TARGET_NAME} ${SOURCES})
list(APPEND INC_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LINK_LIBRARIES ${SDL2_LIBRARIES})

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
