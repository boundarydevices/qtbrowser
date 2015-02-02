TEMPLATE = app
TARGET = stronglink

QT += webkit network

QMAKE_CXXFLAGS += -DSTRONGLINK_TEST

# Input
HEADERS += ../stronglink.h
SOURCES += ../stronglink.cpp

CONFIG += qt warn_on release

target.path = /usr/bin
INSTALLS += target
