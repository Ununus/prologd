#-------------------------------------------------
#
# Project created by QtCreator 2017-01-04T06:16:33
#
#-------------------------------------------------

QT += core gui widgets

TARGET = Prolog-D
TEMPLATE = app

SOURCES += \
    gui/Graphics.cpp \
    gui/MainWindow.cpp \
    gui/Actions.cpp \
    gui/Editor.cpp \
    gui/TextFinder.cpp \
    gui/AboutProgram.cpp \
    gui/Highlighter.cpp \
    gui/Settings.cpp \
    PrologD.cpp \
    preprocessor.cpp \
    prlib/control.cpp \
    prlib/extfunc.cpp \
    prlib/err.cpp \
    prlib/functions.cpp \
    prlib/pstructs.cpp \
    prlib/scaner.cpp

HEADERS  += \
    gui/Graphics.h \
    gui/MainWindow.h \
    gui/Actions.h \
    gui/Editor.h \
    gui/TextFinder.h \
    gui/AboutProgram.h \
    gui/Highlighter.h \
    gui/Settings.h \
    PrologD.h \
    preprocessor.h \
    prlib/control.h \
    prlib/extfunc.h \
    prlib/err.h \
    prlib/functions.h \
    prlib/pdefs.h \
    prlib/pstructs.h \
    prlib/scaner.h

INCLUDEPATH += $$PWD/prlib/3rdparty/boost/include

RESOURCES += res.qrc

win32: RC_ICONS += images/prolog1.ico
