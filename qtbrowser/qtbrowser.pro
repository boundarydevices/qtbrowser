TEMPLATE = app
TARGET = qtbrowser

QT += webkit network widgets webkitwidgets

# Input
HEADERS += ../accelInput.h ../bcInput.h ../compassInput.h ../gpsInput.h ../kbdInput.h ../magstripe.h ../mainwindow.h ../print.h ../process.h ../rfid.h ../stronglink.h ../three_axis_input.h ../RRWebPage.h
SOURCES += ../accelInput.cpp ../bcInput.cpp ../compassInput.cpp ../gpsInput.cpp ../kbdInput.cpp ../magstripe.cpp ../main.cpp ../mainwindow.cpp ../print.cpp ../process.cpp ../rfid.cpp ../stronglink.cpp ../three_axis_input.cpp ../RRWebPage.cpp

CONFIG += qt warn_on release

target.path = /usr/bin
INSTALLS += target
