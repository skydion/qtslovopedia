# -------------------------------------------------
# Project created by QtCreator 2010-02-14T11:31:25
# -------------------------------------------------
TARGET = qtSlovopedia
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    scanthread.cpp
HEADERS += mainwindow.h \
    scanthread.h
FORMS += mainwindow.ui
RESOURCES += resource.qrc
QT += sql \
    network \
    widgets \
    gui \
    xml
QT -= activeqt
CONFIG += debug_and_release
OTHER_FILES += crossword.sql \
    slovopedia.sql
