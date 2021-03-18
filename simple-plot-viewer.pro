include(lib/plot/plot.pri)

VERSION = 1.1
QT += widgets

CONFIG += c++11

TARGET = simple-plot
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    tablewindow.cpp

HEADERS  += \
    mainwindow.h \
    tablewindow.h

FORMS += \
    mainwindow.ui \
    tablewindow.ui
