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

SOURCES += main.cpp\
    mangaeditor.cpp \
    kmm.cpp \
    preferences.cpp

HEADERS  += \
    mangaeditor.h \
    kmm.h \
    preferences.h

FORMS    += \
    mangaeditor.ui \
    kmm.ui \
    preferences.ui

DISTFILES += \
    TODO_LIST
