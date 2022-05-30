cmake_minimum_required (VERSION 3.8)

project ("prologd")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_GUI "Build GUI editor" ON)

add_subdirectory("prlib")

if(BUILD_GUI)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Gui Core REQUIRED)

set (SOURCES
"PrologD.cpp" "PrologD.h"
"gui/AboutProgram.cpp" "gui/AboutProgram.h"
"gui/Actions.cpp" "gui/Actions.h"
"gui/Editor.cpp" "gui/Editor.h"
"gui/Graphics.cpp" "gui/Graphics.h"
"gui/Highlighter.cpp" "gui/Highlighter.h"
"gui/MainWindow.cpp" "gui/MainWindow.h"
"gui/Settings.cpp" "gui/Settings.h"
"gui/TextFinder.cpp" "gui/TextFinder.h")

qt5_add_resources(SOURCES res.qrc)

set(APP_ICON_RES "icon.rc")

add_executable (Prolog-D ${SOURCES} ${APP_ICON_RES})

target_link_libraries(Prolog-D prlib)
target_link_libraries(Prolog-D Qt5::Widgets)
endif()

add_executable(prologd "prologdc.cpp")
target_link_libraries(prologd prlib)
