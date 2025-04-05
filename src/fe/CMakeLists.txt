cmake_minimum_required(VERSION 3.7.2)

project(fod)
set(TARGET_NAME fod)

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
