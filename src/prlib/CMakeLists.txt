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

set_property(TARGET prlib PROPERTY CXX_STANDARD 17)
set_property(TARGET prlib PROPERTY CXX_STANDARD_REQUIRED 17)
set_property(TARGET prlib PROPERTY CXX_EXTENSIONS Off)

target_include_directories(prlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/boost/include)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(prlib -static-libgcc -static-libstdc++)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
