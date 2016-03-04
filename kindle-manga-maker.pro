#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T08:27:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kindle-manga-maker
TEMPLATE = app
CONFIG += c++11
DEFINES *= QT_USE_QSTRINGBUILDER

SOURCES += main.cpp\
    mangaeditor.cpp \
    kmm.cpp \
    preferences.cpp \
    about.cpp \
    projarchive.cpp

HEADERS  += \
    mangaeditor.h \
    kmm.h \
    preferences.h \
    about.h \
    projarchive.h

FORMS    += \
    mangaeditor.ui \
    kmm.ui \
    preferences.ui \
    about.ui

DISTFILES += \
    TODO_LIST

LIBS += -lzip

RESOURCES += \
    resources.qrc
