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
    projarchive.cpp \
    imageviewer.cpp \
    picanalyzeresult.cpp \
    bookexporter.cpp \
    runexternalprocess.cpp

HEADERS  += \
    mangaeditor.h \
    kmm.h \
    preferences.h \
    about.h \
    projarchive.h \
    imageviewer.h \
    picanalyzeresult.h \
    bookexporter.h \
    runexternalprocess.h

FORMS    += \
    mangaeditor.ui \
    kmm.ui \
    preferences.ui \
    about.ui \
    imageviewer.ui \
    picanalyzeresult.ui \
    runexternalprocess.ui

DISTFILES += \
    README.md \
    TODO_LIST

LIBS += -lzip

RESOURCES += \
    resources.qrc
