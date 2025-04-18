cmake_minimum_required(VERSION 3.16.3)

option(ENABLE_TOOLS "Enable developer tools" OFF)
option(ENABLE_TESTS "Enable tests" OFF)
option(ENALBE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

# This code uses "insecure" (as determined by Microsoft) CRT functions
# Disable warnings about this across the board.
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(lib)
add_subdirectory(fe)
add_subdirectory(installer)
if (ENABLE_TOOLS)
  add_subdirectory(tools)
endif()
#if (ENABLE_TESTS)
#  add_subdirectory(tests)
#endif()
