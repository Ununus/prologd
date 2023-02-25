cmake_minimum_required (VERSION 3.8)

project ("prlib")

add_library (prlib 
"control.cpp" "control.h"
"err.cpp" "err.h"
"extfunc.cpp" "extfunc.h"
"functions.cpp" "functions.h"
"pstructs.cpp" "pstructs.h"
"scaner.cpp" "scaner.h"
"pdefs.h"
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(prlib -static-libgcc -static-libstdc++)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
