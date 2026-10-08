cmake_minimum_required (VERSION 3.8)

project ("prologd")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

add_subdirectory("prlib")

add_executable(prologd "prologdc.cpp" "preprocessor.h" "preprocessor.cpp")
target_link_libraries(prologd prlib)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(prologd -static-libgcc -static-libstdc++)
endif()
