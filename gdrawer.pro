######################################################################
# Automatically generated by qmake (2.01a) ?? ????. 14 02:59:35 2013
######################################################################
CONFIG += c++11
TEMPLATE = app
TARGET = gdrawer
DEPENDPATH += . src
INCLUDEPATH += . src
macx {
	INCLUDEPATH += /usr/local/Cellar/boost/1.53.0/include
}
win32 {
	INCLUDEPATH += x:/boost
}
QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra
QMAKE_CXXFLAGS_RELEASE += -std=c++11 -Wall -Wextra
CONFIG += debug
QT += widgets
# Input
HEADERS += src/gdrawer.hpp
SOURCES += src/parse.cpp src/vm.cpp src/main.cpp src/ui.cpp src/draw.cpp
RESOURCES += gdrawer.qrc
