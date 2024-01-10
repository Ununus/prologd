cmake_minimum_required (VERSION 3.8)

project ("prologd")


set(CMAKE_CXX_STANDARD 17)
option(BUILD_GUI "Build GUI editor" OFF)

add_subdirectory("prlib")

if(BUILD_GUI)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Gui Core REQUIRED)

set (SOURCES
"PrologD.cpp" "PrologD.h"
"gui/AboutProgram.cpp" "gui/AboutProgram.h"
"gui/Actions.cpp" "gui/Actions.h"
"gui/Editor.cpp" "gui/Editor.h"
"gui/Graphics.cpp" "gui/Graphics.h"
"gui/Highlighter.cpp" "gui/Highlighter.h"
"gui/MainWindow.cpp" "gui/MainWindow.h"
"gui/Settings.cpp" "gui/Settings.h"
"gui/TextFinder.cpp" "gui/TextFinder.h"
"preprocessor.h" "preprocessor.cpp")

qt6_add_resources(SOURCES res.qrc)

set(APP_ICON_RES "icon.rc")
if (WIN32)
add_executable (Prolog-D WIN32 ${SOURCES} ${APP_ICON_RES})
elseif (APPLE)
set(icon_filename "prolog1.ico")
set(MACOSX_BUNDLE_ICON_FILE ${icon_filename})
set(icon_path "images/${icon_filename}")
set_source_files_properties(${icon_path} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
qt6_add_executable(Prolog-D MANUAL_FINALIZATION MACOSX_BUNDLE ${SOURCES} ${icon_path})
else()
add_executable (Prolog-D ${SOURCES} ${APP_ICON_RES})
endif()

target_link_libraries(Prolog-D PRIVATE prlib)
target_link_libraries(Prolog-D PRIVATE Qt6::Widgets)

if (APPLE)
qt6_finalize_target(Prolog-D)
endif()

add_custom_command(TARGET Prolog-D POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/src/prologd-help.html ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_executable(prologd "prologdc.cpp" "preprocessor.h" "preprocessor.cpp")
target_link_libraries(prologd prlib)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(prologd -static-libgcc -static-libstdc++)
endif()
