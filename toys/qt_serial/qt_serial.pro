CONFIG += console debug
QT += serialport widgets gui

SOURCES += main_window.cc
HEADERS += main_window.h

FORMS += main_window.ui

QMAKE_CXXFLAGS += --std=c++11
