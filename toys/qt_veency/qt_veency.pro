CONFIG += console debug
QT += widgets gui

SOURCES += MainWindow.cc
HEADERS += MainWindow.h

FORMS += MainWindow.ui

QMAKE_CXXFLAGS += --std=c++11
QMAKE_LFLAGS += -lvncclient
