cmake_minimum_required (VERSION 3.8)

project ("prologd")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (prologd "prologdc.cpp")

add_subdirectory("prlib")
target_link_libraries(prologd prlib)